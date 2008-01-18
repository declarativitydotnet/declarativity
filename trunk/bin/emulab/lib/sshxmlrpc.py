#! /usr/bin/env python

#
# EMULAB-COPYRIGHT
# Copyright (c) 2004 University of Utah and the Flux Group.
# All rights reserved.
#
# Permission to use, copy, modify and distribute this software is hereby
# granted provided that (1) source code retains these copyright, permission,
# and disclaimer notices, and (2) redistributions including binaries
# reproduce the notices in supporting documentation.
#
# THE UNIVERSITY OF UTAH ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  THE UNIVERSITY OF UTAH DISCLAIMS ANY LIABILITY OF ANY KIND
# FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
#
##########################################################################
# Some bits of this file are from xmlrpclib.py, which is:
# --------------------------------------------------------------------
# Copyright (c) 1999-2002 by Secret Labs AB
# Copyright (c) 1999-2002 by Fredrik Lundh
#
# By obtaining, using, and/or copying this software and/or its
# associated documentation, you agree that you have read, understood,
# and will comply with the following terms and conditions:
#
# Permission to use, copy, modify, and distribute this software and
# its associated documentation for any purpose and without fee is
# hereby granted, provided that the above copyright notice appears in
# all copies, and that both that copyright notice and this permission
# notice appear in supporting documentation, and that the name of
# Secret Labs AB or the author not be used in advertising or publicity
# pertaining to distribution of the software without specific, written
# prior permission.
#
# SECRET LABS AB AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
# TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANT-
# ABILITY AND FITNESS.  IN NO EVENT SHALL SECRET LABS AB OR THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
# DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
# ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.
# --------------------------------------------------------------------
#
import sys
import types
import urllib
import popen2
import rfc822
import xmlrpclib
import os, os.path

VERSION = 0.5

##
## BEGIN Debugging setup
##

SSHXMLRPC_DEBUG = os.environ.get("SSHXMLRPC_DEBUG", "")

# SSHXMLRPC_DEBUG = "all"

if "all" in SSHXMLRPC_DEBUG:
    SSHXMLRPC_DEBUG = "config,connect,io,ssh"
    pass

def __sxrdebug__(key, *args):
    if key in SSHXMLRPC_DEBUG:
        sys.stderr.write("sshxmlrpc.py: " + "".join(args) + "\n")
        pass
    return

##
## END Debugging setup
##



__sxrdebug__("config", "OS ", os.name)

if os.name != "nt":
    import syslog
    LOG_TESTBED = syslog.LOG_LOCAL5;
    pass

import traceback



##
## BEGIN Self-configuration
##

##
# Search the user's PATH for the given command.
#
# @param command The command name to search for.
# @return The full path to the command or None if the command was not found.
#
def conf_which(command):
    if os.path.exists(command) and os.path.isfile(command):
        return command
    for path in os.environ.get('PATH', os.defpath).split(os.pathsep):
        fullpath = os.path.join(path, command)
        if os.path.exists(fullpath) and os.path.isfile(fullpath):
            return fullpath
        pass
    return None

##
# Search for several commands in the user's PATH and return the first match.
#
# @param possible_commands The list of commands to search for.
# @return The first command that matched, or None if no match was found.
#
def conf_detect(possible_commands):
    for cmd in possible_commands:
        if len(cmd) > 0:
            cmd_no_flags = cmd.split()[0]
            retval = conf_which(cmd_no_flags)
            if retval is not None:
                return cmd
            pass
        pass
    return None

# The default identity for the user.
DEFAULT_SSH_IDENTITY = os.path.join(os.path.expanduser("~"),
                                    ".ssh",
                                    "identity")

__sxrdebug__("config", "DEFAULT_SSH_IDENTITY - ", str(DEFAULT_SSH_IDENTITY))

##
# Check if the user can perform a passphrase-less login.
#
# @param identity The identity to check.
# @return True if the user can perform a passphraseless login.
#
def conf_passphraseless_login(identity=None):
    if os.environ.get("SSH_AUTH_SOCK", "") == "":
        # No agent, check for a passphrase-less key and then
        if SSHKEYGEN_COMMAND is not None:
            if not identity or (identity == ""):
                identity = DEFAULT_SSH_IDENTITY
                pass
            rc = os.system(SSHKEYGEN_COMMAND
                           + " -p -P \"\" -N \"\" -f "
                           + identity
                           + " > /dev/null 2>&1")
            if rc != 0:
                retval = False
                pass
            else:
                retval = True
                pass
            pass
        # ... complain.
        elif os.name != "nt":
            retval = False
            pass
        pass
    else:
        retval = True
        pass
    
    return retval

# Find a suitable "ssh" command and
SSH_COMMAND = conf_detect([
    os.environ.get("SSHXMLRPC_SSH", ""),
    "ssh -T -x -C -o 'CompressionLevel 5' %(-F)s %(-l)s %(-i)s %(-v)s %(-1)s %(-2)s",
    "plink -x -C %(-l)s %(-i)s %(-v)s %(-1)s %(-2)s",
    ])

__sxrdebug__("config", "SSH_COMMAND - ", str(SSH_COMMAND))

# ... error out if we don't.
if SSH_COMMAND is None:
    sys.stderr.write("sshxmlrpc.py: Unable to locate a suitable SSH command\n")
    if os.environ.has_key("SSHXMLRPC_SSH"):
        sys.stderr.write("sshxmlrpc.py: '"
                         + os.environ["SSHXMLRPC_SSH"]
                         + "' was not found.\n")
        pass
    else:
        sys.stderr.write("sshxmlrpc.py: Set the SSHXMLRPC_SSH environment "
                         "variable to a suitable binary (e.g. ssh/plink)\n")
        pass
    raise ImportError, "suitable ssh not found in path: %(PATH)s" % os.environ

# Find ssh-keygen so we can do some tests when a connection is made.
SSHKEYGEN_COMMAND = conf_detect([
    os.environ.get("SSHXMLRPC_SSHKEYGEN", ""),
    "ssh-keygen",
    ])

__sxrdebug__("config", "SSHKEYGEN_COMMAND - ", str(SSHKEYGEN_COMMAND))

##
## END Self-configuration
##



##
# Base class for exceptions in this module.
#
class SSHException(Exception):
    pass

##
# Indicates a poorly formatted response from the server.
#
class BadResponse(SSHException):

    ##
    # @param host The server host name.
    # @param handler The handler being accessed on the server.
    # @param arg Description of the problem.
    #
    def __init__(self, host, handler, msg):
        self.args = host, handler, msg,
        return
    
    pass

##
# Indicates a poorly formatted request from the client.
#
class BadRequest(SSHException):

    ##
    # @param host The client host name.
    # @param arg Description of the problem.
    #
    def __init__(self, host, msg):
        self.args = host, msg,
        return
    
    pass

##
# Class used to decode headers.
#
class SSHMessage(rfc822.Message):
    pass

##
# An SSH based connection class.
#
class SSHConnection:

    ##
    # @param host The peer host name.
    # @param handler The handler being accessed.
    # @param streams A pair containing the input and output files respectively.
    # If this value is not given, ssh will be used to connect to the host.
    # @param ssh_config The ssh config file to use when initiating a new
    # connection.
    # 
    def __init__(self, host, handler, streams=None, ssh_config=None,
                 ssh_identity=None, ssh_opts={}):
        # Store information about the peer and
        self.handler = handler
        self.host = host
        self.last_lines = []

        # ... initialize the read and write file objects.
        if streams:
            self.rfile = streams[0]
            self.wfile = streams[1]
            self.errfile = None
            self.closed = False
            pass
        else:
            if not conf_passphraseless_login(ssh_identity):
                sys.stderr.write("sshxmlrpc.py: warning - No agent or "
                                 "passphrase-less key found, "
                                 "continuing anyways...\n")
                pass
            
            self.user, ssh_host = urllib.splituser(self.host)

            all_opts = { "-l" : "", "-i" : "", "-F" : "", "-v" : "",
                         "-1" : "", "-2" : "" }
            all_opts.update(ssh_opts)
            if self.user:
                all_opts["-l"] = "-l " + self.user
                pass
            if ssh_identity:
                all_opts["-i"] = "-i " + ssh_identity
                pass
            if ssh_config:
                all_opts["-F"] = "-F " + ssh_config
                pass
            if "ssh" in SSHXMLRPC_DEBUG:
                all_opts["-v"] = "-vvv"
                pass
            
            args = (SSH_COMMAND % all_opts) + " " + ssh_host + " " + handler
            
            __sxrdebug__("connect", "open - ", args)
            
            # Open the pipe in Binary mode so it doesn't mess with CR-LFs.
            self.rfile, self.wfile, self.errfile = popen2.popen3(
                args, mode='b')
            self.closed = False
            pass
        
        return

    ##
    # @param len The amount of data to read. (Default: 1024)
    # @return The amount of data read.
    #
    def read(self, len=1024):
        retval = self.rfile.read(len)

        __sxrdebug__("io", "read - ", retval)
        return retval

    ##
    # @return A line of data or None if there is no more input.
    #
    def readline(self):
        retval = self.rfile.readline()
        if len(retval) > 0:
            self.last_lines.append(retval)
            if len(self.last_lines) > 5:
                self.last_lines.pop(0)
                pass
            pass
        else:
            self.closed = True
            pass
        
        __sxrdebug__("io", "readline - ", retval)
        return retval

    ##
    # @param stuff The data to send to the other side.
    # @return The amount of data written.
    #
    def write(self, stuff):
        __sxrdebug__("io", "write - ", stuff)
        
        return self.wfile.write(stuff)

    ##
    # Flush any write buffers.
    #
    def flush(self):
        __sxrdebug__("io", "flush")
        
        self.wfile.flush()
        return

    ##
    # Close the connection.
    #
    def close(self):
        __sxrdebug__("connect", "close - ", self.host)
        
        self.wfile.close()
        self.rfile.close()
        return

    ##
    # Send an rfc822 style header to the other side.
    #
    # @param key The header key.
    # @param value The value paired with the given key.
    #
    def putheader(self, key, value):
        self.write("%s: %s\r\n" % (key, str(value)))
        return

    ##
    # Terminate the list of headers so the body can follow.
    #
    def endheaders(self):
        self.write("\r\n")
        self.flush()
        return

    ##
    # Dump the standard error from the peer to the given file pointer with the
    # given prefix.
    #
    # @param fp The file pointer where the output should be written or None if
    # you just want to drain the pipe.
    # @param prefix Prefix to prepend to every line. (optional)
    #
    def dump_stderr(self, fp, prefix=""):
        if self.errfile:
            while True:
                line = self.errfile.readline()
                if not line:
                    break
                if fp:
                    fp.write(prefix + line)
                    pass
                pass
            pass
        return

    ##
    # Dump the last five lines of input read from the peer.  Helpful for
    # debugging connections and what not.
    #
    # @param fp The file pointer where the output should be written.
    # @param prefix Prefix to prepend to every line. (optional)
    #
    def dump_last_lines(self, fp, prefix=""):
        if len(self.last_lines) < 5:
            for lpc in range(len(self.last_lines), 5):
                if not self.readline():
                    break
                pass
            pass
        for line in self.last_lines:
            fp.write(prefix + line)
            pass
        return

    def __repr__(self):
        return "<SSHConnection %s%s>" % (self.host, self.handler)

    __str__ = __repr__

    pass

##
# Use SSH to transport XML-RPC requests/responses
#
class SSHTransport:

    ##
    # @param ssh_config The ssh config file to use when making new connections.
    # @param user_agent Symbolic name for the program acting on behalf of the
    #   user.
    # @param ssh_opts List of additional options to pass to SSH_COMMAND.
    #
    def __init__(self, ssh_config=None, user_agent=None, ssh_identity=None,
                 ssh_opts={}):
        self.connections = {}
        self.ssh_config = ssh_config
        if user_agent:
            self.user_agent = user_agent
            pass
        else:
            self.user_agent = sys.argv[0]
            pass
        self.ssh_identity = ssh_identity
        self.ssh_opts = ssh_opts
        return

    ##
    # Probe the peer and return their response headers.  Useful for making sure
    # the other side is what we expect it to be.
    #
    # @param host The host to contact.
    # @param handler The XML-RPC handler.
    # @param hdrs A dictionary of additional headers to send to the peer, these
    # will be included in their response.
    # @return The response headers from the peer.
    # @throws BadResponse if there was a problem interpreting the other side's
    # response.
    #
    def probe(self, host, handler, hdrs={}, verbose=False):
        handler = self.munge_handler(handler)
        
        connection = self.get_connection((host, handler))
        connection.putheader("probe", self.user_agent)
        for (key, value) in hdrs.items():
            connection.putheader(key, str(value))
            pass
        connection.endheaders()
        connection.flush()

        return self.parse_headers(connection, verbose=verbose)

    ##
    # Send a request to the destination.
    #
    # @param host The host name on which to execute the request
    # @param handler The python file that will handle the request.
    # @param request_body The XML-RPC encoded request.
    # @return The value returned by the peer method.
    #
    def request(self, host, handler, request_body, path=None):
        handler = self.munge_handler(handler, path)

        # Try to get a new connection,
        connection = self.get_connection((host,handler))

        # ... send our request, and
        connection.putheader("user-agent", self.user_agent)
        connection.putheader("content-length", len(request_body))
        connection.putheader("content-type", "text/xml")
        connection.endheaders()
        connection.write(request_body)
        connection.flush()

        # ... parse the response.
        retval = self.parse_response(connection)

        return retval

    ##
    # @return A tuple containing the parser and unmarshaller, in that order
    #
    def getparser(self):
        return xmlrpclib.getparser()

    ##
    # Munge the handler which means stripping the first slash, if it is there.
    #
    # @param handler The handler to munge.
    # @return The munged handler string.
    #
    def munge_handler(self, handler, path=None):
        # Strip the leading slash in the handler, if there is one.
        if path:
            retval = path + handler
            pass
        elif handler.startswith('/'):
            retval = handler[1:]
            pass
        else:
            retval = handler
            pass
        
        return retval

    ##
    # Get a cached connection or make a new one.
    #
    # @param pair The host/handler pair that identifies the connection.
    # @return An SSHConnection object for the given pair.
    #
    def get_connection(self, pair):
        if not self.connections.has_key(pair):
            __sxrdebug__("connect",
                         "new connection for ", pair[0], " ", pair[1])
            
            self.connections[pair] = SSHConnection(
                pair[0], pair[1],
                ssh_identity=self.ssh_identity, ssh_opts=self.ssh_opts)
            pass
        return self.connections[pair]

    ##
    # @param connection The connection to drop.
    #
    def drop_connection(self, connection):
        del self.connections[(connection.host,connection.handler)]
        connection.close()
        return

    ##
    # Parse the headers from the peer.
    #
    # @param connection The connection to read the headers from.
    # @param verbose Be verbose in providing error information. (default: True)
    #
    def parse_headers(self, connection, verbose=True):
        retval = SSHMessage(connection, False)
        if retval.status != "":
            if verbose:
                connection.dump_stderr(sys.stderr,
                                       connection.host + ",stderr: ")
                sys.stderr.write("sshxmlrpc.py: Error while reading headers, "
                                 "expected rfc822 headers, received:\n")
                connection.dump_last_lines(sys.stderr, ">> ")
                pass
            self.drop_connection(connection)
            raise BadResponse(connection.host,
                              connection.handler,
                              retval.status)
        
        return retval
    
    ##
    # Parse the response from the server.
    #
    # @return The python value returned by the server method.
    #
    def parse_response(self, connection):
        parser, unmarshaller = self.getparser()

        try:
            # Get the headers,
            headers = self.parse_headers(connection)
            
            # ... the length of the body, and
            length = int(headers['content-length'])
            # ... read in the body.
            response = connection.read(length)
        except KeyError, e:
            connection.dump_stderr(sys.stderr, connection.host + ",stderr: ")
            sys.stderr.write("sshxmlrpc.py: Error while reading headers, "
                             + "expected rfc822 headers, received:\n")
            connection.dump_last_lines(sys.stderr, ">> ")
            # Bad header, drop the connection, and
            self.drop_connection(connection)
            # ... tell the user.
            raise BadResponse(connection.host, connection.handler, e.args[0])

        parser.feed(response)

        return unmarshaller.close()
    
    pass


##
# A wrapper for objects that should be exported via an XML-RPC interface.  Any
# method calls received via XML-RPC will automatically be translated into real
# python calls.
#
class SSHServerWrapper:

    ##
    # Initialize this object.
    #
    # @param object The object to wrap.
    # @param probe_response The value to send back to clients in the
    # 'probe-response' header.
    #
    def __init__(self, object, probe_response=None):
        self.ssh_connection = os.environ.get(
            "SSH_CONNECTION", "stdin 0 stdout 0").split()
        self.myObject = object
        self.probe_response = probe_response
        if self.probe_response is None:
            self.probe_response = sys.argv[0] + " " + str(VERSION)
            pass

        #
        # Init syslog
        #
        if os.name != "nt":
            syslog.openlog("sshxmlrpc", syslog.LOG_PID, LOG_TESTBED);
            syslog.syslog(syslog.LOG_INFO,
                          "Connect by " + os.environ['USER'] + " from " +
                          self.ssh_connection[0]);
            pass
        
        return

    ##
    # Handle a single request from a client.  The method will read the request
    # from the client, dispatch the method, and write the response back to the
    # client.
    #
    # @param connection An initialized SSHConnection object.
    #
    def handle_request(self, connection):
        retval = False
        try:
            # Read the request headers,
            hdrs = SSHMessage(connection, False)

            # ... make sure they are sane,
            if hdrs.status != "":
                if not connection.closed:
                    sys.stderr.write("server error: Expecting rfc822 headers, "
                                     "received:\n");
                    connection.dump_last_lines(sys.stderr, "<< ")
                    sys.stderr.write("conn " + `connection.closed` + "\n")
                    sys.stderr.write("server error: " + hdrs.status)
                    raise BadRequest(connection.host, hdrs.status)
                else:
                    return True
                pass

            # ... respond to probes immediately,
            if hdrs.has_key("probe"):
                connection.putheader("probe-response", self.probe_response)
                del hdrs["content-length"]
                connection.putheader("content-length", 0)
                for (key, value) in hdrs.items():
                    connection.putheader(key, value)
                    pass
                connection.endheaders()
                connection.flush()
                return retval

            # ... check for required headers, and
            if not hdrs.has_key('content-length'):
                sys.stderr.write("server error: expecting content-length "
                                 "header, received:\n")
                connection.dump_last_lines(sys.stderr, "<< ")
                raise BadRequest(connection.host,
                                 "missing content-length header")

            if hdrs.has_key('user-agent'):
                user_agent = hdrs['user-agent']
                pass
            else:
                user_agent = "unknown"
                pass
            
            # ... start reading the body.
            length = int(hdrs['content-length'])
            params, method = xmlrpclib.loads(connection.read(length))
            if os.name != "nt":
                syslog.syslog(syslog.LOG_INFO,
                              "Calling method '"
                              + method
                              + "'; user-agent="
                              + user_agent);
            try:
                # ... find the corresponding method in the wrapped object,
                meth = getattr(self.myObject, method)
                # ... dispatch the method, and
                if type(meth) == type(self.handle_request):
                    response = apply(meth, params) # It is really a method.
                    pass
                else:
                    response = str(meth) # Is is just a plain variable.
                    pass
                # ... ensure there was a valid response.
                if type(response) != type((  )):
                    response = (response,)
                    pass
                pass
            except:
                traceback.print_exc()
                # Some other exception happened, convert it to an XML-RPC fault
                response = xmlrpclib.dumps(
                    xmlrpclib.Fault(1,
                                    "%s:%s" % (sys.exc_type, sys.exc_value)))
                pass
            else:
                # Everything worked, encode the real response.
                response = xmlrpclib.dumps(response, methodresponse=1)
                pass
            pass
        except xmlrpclib.Fault, faultobj:
            # An XML-RPC related fault occurred, just encode the response.
            response = xmlrpclib.dumps(faultobj)
            retval = True
            pass
        except:
            # Some other exception happened, convert it to an XML-RPC fault.
            response = xmlrpclib.dumps(
                xmlrpclib.Fault(1, "%s:%s" % (sys.exc_type, sys.exc_value)))
            retval = True
            pass

        # Finally, send the reply to the client.
        connection.putheader("content-length", len(response))
        connection.endheaders()
        connection.write(response)
        connection.flush()

        return retval

    ##
    # Handle all of the user requests.
    #
    # @param streams A pair containing the input and output streams.
    #
    def serve_forever(self, streams):
        # Make a new connection from the streams and handle requests until the
        # streams are closed or there is a protocol error.
        connection = SSHConnection(self.ssh_connection[0], '', streams=streams)
        try:
            done = False
            while not done:
                done = self.handle_request(connection)
                pass
            pass
        finally:
            connection.close()
            if os.name != "nt":
                syslog.syslog(syslog.LOG_INFO, "Connection closed");
                syslog.closelog()
                pass
            pass
        return

    def serve_stdio_forever(self):
        return self.serve_forever((sys.stdin, sys.stdout))
    
    pass


##
# A client-side proxy for XML-RPC servers that are accessible via SSH.
#
class SSHServerProxy:

    ##
    # Initialize the object.
    #
    # @param uri The URI for the server.  Must start with 'ssh'.
    # @param transport A python object that implements the Transport interface.
    # The default is to use a new SSHTransport object.
    # @param encoding Content encoding.
    # @param user_agent Symbolic name for the program acting on behalf of the
    #   user.
    # @param ssh_opts List of additional options to pass to SSH_COMMAND.
    #
    def __init__(self,
                 uri,
                 transport=None,
                 encoding=None,
                 path=None,
                 user_agent=None,
                 ssh_identity=None,
                 ssh_opts={}):
        type, uri = urllib.splittype(uri)
        if type not in ("ssh", ):
            raise IOError, "unsupported XML-RPC protocol: " + `type`

        self.__host, self.__handler = urllib.splithost(uri)

        if transport is None:
            transport = SSHTransport(user_agent=user_agent,
                                     ssh_identity=ssh_identity,
                                     ssh_opts=ssh_opts)
            pass
        
        self.__transport = transport

        self.__encoding = encoding
        self.__path = path
        return

    ##
    # Send a request to the server.
    #
    # @param methodname The name of the method.
    # @param params The parameters for the method.
    #
    def __request(self, methodname, params):
        # Convert the method name and parameters to a string,
        request = xmlrpclib.dumps(params, methodname, encoding=self.__encoding)

        # ... use the transport to send the request and receive the reply, and
        response = self.__transport.request(
            self.__host,
            self.__handler,
            request,
            path=self.__path
            )

        # ... ensure there was a valid reply.
        if len(response) == 1:
            response = response[0]
            pass

        return response

    def __repr__(self):
        return (
            "<ServerProxy for %s%s>" %
            (self.__host, self.__handler)
            )

    __str__ = __repr__

    def __getattr__(self, name):
        # magic method dispatcher
        return xmlrpclib._Method(self.__request, name)

    # Locally handle "if not server:".
    def __nonzero__(self):
        return True

    pass

if __name__ == "__main__":
    import time
    import getopt

    def usage():
        print "SSH-based XML-RPC module/client."
        print "Usage: sshxmlrpc.py [-hVq] [-u agent] [-i id] [-s opts] [<URL>]"
        print "       sshxmlrpc.py [-u agent] [-i id] [-s opts] [<URL> <method>]"
        print
        print "Options:"
        print "  -h, --help\t\t  Display this help message"
        print "  -V, --version\t\t  Show the version number"
        print "  -q, --quiet\t\t  Be less verbose"
        print "  -u, --user-agent agent  Specify the user agent"
        print "  -i, --identity id\t  Specify the SSH identity to use"
        print "  -s, --ssh-opts opts\t  Specify additional SSH options"
        print "                     \t  The format is 'opt=value' (e.g. l=stack)"
        print
        print "Required arguments:"
        print "  URL\t\t\t  The URL of the server."
        print "  method\t\t  The method name to call."
        print
        print "Environment Variables:"
        print "  SSHXMLRPC_DEBUG\t  Activate debugging for the listed aspects."
        print "                 \t  (e.g. all,config,connect,io,ssh)"
        print "  SSHXMLRPC_SSH\t\t  Specify the ssh command to use."
        print "  SSHXMLRPC_SSHKEYGEN\t  Specify the ssh-keygen command."
        print
        print "Examples:"
        print "  $ sshxmlrpc.py ssh://localhost/server.py"
        print
        print "Configuration:"
        print "  ssh command\t\t" + SSH_COMMAND
        print "    ( The '%(-X)s' portions of the command are substituted    )"
        print "    ( with the corresponding flags before the command is run. )"
        print "    ( Do not include them if your version of SSH does not     )"
        print "    ( fully support them.                                     )"
        print "  ssh-keygen command\t" + SSHKEYGEN_COMMAND
        return

    verbose = True
    user_agent = None
    ssh_identity = None
    ssh_opts = {}
    
    try:
        opts, extra = getopt.getopt(sys.argv[1:],
                                    "hVqu:i:s:",
                                    [ "help",
                                      "version",
                                      "quiet",
                                      "user-agent=",
                                      "identity=",
                                      "ssh-opts="])

        for opt, val in opts:
            if opt in ("-h", "--help"):
                usage()
                sys.exit()
                pass
            elif opt in ("-V", "--version"):
                print VERSION
                sys.exit()
                pass
            elif opt in ("-q", "--quiet"):
                verbose = False
                pass
            elif opt in ("-u", "--user-agent"):
                user_agent = val
                pass
            elif opt in ("-i", "--identity"):
                ssh_identity = val
                pass
            elif opt in ("-s", "--ssh-opts"):
                so = val.split("=")
                if len(so) == 2:
                    so_key, so_value = so
                    pass
                else:
                    so_key = so[0]
                    so_value = None
                    pass
                so_key = "-" + so_key
                if so_value:
                    ssh_opts[so_key] = so_key + " " + so_value
                    pass
                else:
                    ssh_opts[so_key] = so_key
                    pass
                pass
            else:
                assert not "unhandled option"
                pass
            pass
        pass
    except getopt.error, e:
        print e.args[0]
        usage()
        sys.exit(2)
        pass

    # Print usage if there are no arguments,
    if len(extra) == 0:
        usage()
        sys.exit()
        pass
    # ... check the URL, then
    elif not extra[0].startswith("ssh://"):
        print "Invalid url: " + extra[0]
        usage()
        sys.exit(2)
        pass
    # ... probe the URL or
    elif len(extra) == 1:
        try:
            st = SSHTransport(user_agent=user_agent,
                              ssh_identity=ssh_identity,
                              ssh_opts=ssh_opts)
            type, uri = urllib.splittype(extra[0])
            host, handler = urllib.splithost(uri)
            rc = st.probe(host, handler,
                          { "date" : time.ctime(time.time()) },
                          verbose)
            secs = time.mktime(time.strptime(rc["date"]))
            
            print "Probe results for: " + extra[0]
            print "  response time=%.2f s" % (time.time() - secs)
            print "Response Headers"
            for pair in rc.items():
                print "  %s: %s" % pair
                pass
            pass
        except BadResponse, e:
            print ("sshxmlrpc.py: error - bad response from "
                   + extra[0]
                   + "; "
                   + e[2])
            sys.exit(1)
            pass
        pass
    # ... call a method.
    else:
        try:
            sp = SSHServerProxy(extra[0],
                                ssh_identity=ssh_identity,
                                user_agent=user_agent,
                                ssh_opts=ssh_opts)
            method_name = extra[1]
            method_args = extra[2:]
            method = getattr(sp, method_name)
            print str(apply(method, method_args))
            pass
        except BadResponse, e:
            print ("sshxmlrpc.py: error - bad response from "
                   + extra[0]
                   + "; "
                   + e[2])
            sys.exit(1)
            pass
        pass
    pass
