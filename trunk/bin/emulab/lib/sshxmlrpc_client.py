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

import sys
sys.path.append("/usr/testbed/lib")
import getopt
import os

from sshxmlrpc import *
from emulabclient import *

##
# The package version number
#
PACKAGE_VERSION = 0.1

# Default server
XMLRPC_SERVER   = "boss.emulab.net"

# User supplied server name.
xmlrpc_server   = XMLRPC_SERVER

# User supplied login ID to use (overrides env variable USER if exists).
login_id        = os.environ["USER"]

# The default RPC module to invoke.
module          = "experiment"

# Debugging output.
debug           = 0

#
# For admin people, and for using their devel trees. These options are
# meaningless unless you are an Emulab developer; they will be rejected
# at the server most ungraciously.
#
SERVER_PATH     = "/usr/testbed"
SERVER_DIR      = "sbin"
DEVEL_DIR       = "devel"
develuser       = None
path            = None
admin           = 0
devel           = 0

##
# Print the usage statement to stdout.
#
def usage():
    print "Make a request to the Emulab XML-RPC (SSH-based) server."
    print ("Usage: " + sys.argv[0] 
                     + " [-hV] [-l login] [-s server] [-m module] "
                     + "<method> [param=value ...]")
    print
    print "Options:"
    print "  -h, --help\t\t  Display this help message"
    print "  -V, --version\t\t  Show the version number"
    print "  -s, --server\t\t  Set the server hostname"
    print "  -l, --login\t\t  Set the login id (defaults to $USER)"
    print "  -m, --module\t\t  Set the RPC module (defaults to experiment)"
    print
    print "Required arguments:"
    print "  method\t\t  The method to execute on the server"
    print "  params\t\t\t  The method arguments in param=value format"
    print
    print "Example:"
    print ("  "
           + sys.argv[0]
           + " -s boss.emulab.net echo \"Hello World!\"")
    return

#
# Process a single command line
#
def do_method(server, method_and_args):
    # Get a pointer to the function we want to invoke.
    methodname = method_and_args[0]
    if methodname.count(".") == 0:
        methodname = module + "." + methodname
        pass
    
    meth = getattr(server, methodname)

    # Pop off the method, and then convert the rest of the arguments.
    # Be sure to add the version.
    method_and_args.pop(0)

    #
    # Convert all params (name=value) into a Dictionary. 
    # 
    params = {}
    for param in method_and_args:
        plist = string.split(param, "=", 1)
        if len(plist) != 2:
            print ("error: Parameter, '"
                   + param
                   + "', is not of the form: param=value!")
            return -1
        value = plist[1]

        #
        # If the first character of the argument looks like a dictionary,
        # try to evaluate it.
        #
        if value.startswith("{"):
            value = eval(value);
            pass
    
        params[plist[0]] = value
        pass
    meth_args = [ PACKAGE_VERSION, params ]

    #
    # Make the call. 
    #
    try:
        response = apply(meth, meth_args)
        pass
    except BadResponse, e:
        print ("error: bad reponse from host, " + e.args[0]
               + ", and handler: " + e.args[1])
        print "error: " + e.args[2]
        return -1
    except xmlrpclib.Fault, e:
        print e.faultString
        return -1

    #
    # Parse the Response, which is a Dictionary. See EmulabResponse in the
    # emulabclient.py module. The XML standard converts classes to a plain
    # Dictionary, hence the code below. 
    # 
    if len(response["output"]):
        print response["output"]
        pass

    rval = response["code"]

    #
    # If the code indicates failure, look for a "value". Use that as the
    # return value instead of the code. 
    # 
    if rval != RESPONSE_SUCCESS:
        if response["value"]:
            rval = response["value"]
            pass
        pass

    if debug and response["value"]:
        print str(response["value"])
        pass
        
    return rval

#
# Process program arguments.
# 
try:
    # Parse the options,
    opts, req_args =  getopt.getopt(sys.argv[1:],
                      "dhVs:l:am:zx:",
                      [ "help", "version", "server=", "login=", "module="])
    # ... act on them appropriately, and
    for opt, val in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
            pass
        elif opt in ("-V", "--version"):
            print PACKAGE_VERSION
            sys.exit()
            pass
        elif opt in ("-s", "--server"):
	    xmlrpc_server = val
            pass
        elif opt in ("-l", "--login"):
	    login_id = val
            pass
        elif opt in ("-m", "--module"):
	    module = val
            pass
        elif opt in ("-d", "--debug"):
	    debug = 1
            pass
        elif opt in ("-a", "--admin"):
	    admin = 1
            pass
        elif opt in ("-z", "--devel"):
	    devel = 1
            pass
        elif opt in ("-x", "--develuser"):
	    develuser = val
            pass
        pass
    pass
except getopt.error, e:
    print e.args[0]
    usage()
    sys.exit(2)
    pass

# This is parsed by the Proxy object.
URI = "ssh://" + login_id + "@" + xmlrpc_server + "/xmlrpc";

# All of this admin cruft is totally ignored if you are not a testbed admin.
if admin:
    path = SERVER_PATH
    if devel:
        path += "/" + DEVEL_DIR
        if develuser:
            path += "/" + develuser
            pass
        else:
            path += "/" + login_id
            pass
        pass
    path += "/" + SERVER_DIR
    URI +=  "/server"
    pass

# Get a handle on the server,
server = SSHServerProxy(URI, path=path,
                        user_agent="sshxmlrpc_client-v0.1")

if len(req_args):
    # Method and args are on the command line.
    sys.exit(do_method(server, req_args))
else:
    # Prompt the user for input.
    try:
        while True:
            line = raw_input("$ ")
            tokens = line.split(" ")
            if len(tokens) >= 1 and len(tokens[0]) > 0:
                print str(do_method(server, tokens))
                pass
            pass
        pass
    except EOFError:
        pass
    print
    pass

