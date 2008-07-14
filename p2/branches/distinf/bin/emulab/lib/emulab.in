#! /usr/bin/env python
#
# EMULAB-COPYRIGHT
# Copyright (c) 2004, 2005, 2006, 2007 University of Utah and the Flux Group.
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

#
# Wrapper to convert select commands into XMLRPC calls to boss. The point
# is to provide a script interface that is backwards compatable with the
# pre-rpc API, but not have to maintain that interface beyond this simple
# conversion.
#
import sys
import pwd
import getopt
import os
import re

import xmlrpclib
from sshxmlrpc import *
from emulabclient import *

##
# The package version number
#
PACKAGE_VERSION = 0.1

# Default server
XMLRPC_SERVER   = "boss.emulab.net"
XMLRPC_PORT   = 3069
SERVER_PATH     = "/usr/testbed"

# User supplied server name.
xmlrpc_server   = XMLRPC_SERVER

# User supplied login ID to use (overrides env variable USER if exists).
login_id        = os.environ["USER"]

# Debugging output.
debug           = 0
impotent        = 0

try:
    pw = pwd.getpwuid(os.getuid())
    pass
except KeyError:
    sys.stderr.write("error: unknown user id %d" % os.getuid())
    sys.exit(2)
    pass

USER = pw.pw_name
HOME = pw.pw_dir

CERTIFICATE = os.path.join(HOME, ".ssl", "emulab.pem")

API = {
    "node_reboot"       : { "func" : "reboot",
                            "help" : "Reboot selected nodes or all nodes in " +
                                     "an experiment" },
    "node_list"         : { "func" : "node_list",
                            "help" : "Print physical mapping of nodes " +
                                     "in an experiment" },
    "node_avail"        : { "func" : "node_avail",
                            "help" : "Print free node counts" },
    "delay_config"      : { "func" : "delay_config",
                            "help" : "Change the link shaping characteristics " +
                                     "for a link or lan" },
    "wilink_config"     : { "func" : "wilink_config",
                            "help" : "Change INTERFACE parameters " +
                                     "for a WIRELESS link" },
    "savelogs"          : { "func" : "savelogs",
                            "help" : "Save console tip logs to experiment " +
                                     "directory" },
    "portstats"         : { "func" : "portstats",
                            "help" : "Get portstats from the switches" },
    "eventsys_control"  : { "func" : "eventsys_control",
                            "help" : "Start/Stop/Restart the event system" },
    "readycount"        : { "func" : "readycount",
                            "help" : "Get readycounts for nodes in experiment " +
                                     "(deprecated)" },
    "nscheck"           : { "func" : "nscheck",
                            "help" : "Check and NS file for parser errors" },
    "startexp"          : { "func" : "startexp",
                            "help" : "Start an Emulab experiment" },
    "batchexp"          : { "func" : "startexp",
                            "help" : "Synonym for startexp" },
    "swapexp"           : { "func" : "swapexp",
                            "help" : "Swap experiment in or out" },
    "modexp"            : { "func" : "modexp",
                            "help" : "Modify experiment" },
    "endexp"            : { "func" : "endexp",
                            "help" : "Terminate an experiment" },
    "expinfo"           : { "func" : "expinfo",
                            "help" : "Get information about an experiment" },
    "tbuisp"            : { "func" : "tbuisp",
                            "help" : "Upload code to a mote" },
    "expwait"           : { "func" : "expwait",
                            "help" : "Wait for experiment to reach a state" },
    "tipacl"            : { "func" : "tipacl",
                            "help" : "Get console acl" },
    "template_commit"   : { "func" : "template_commit",
                            "help" : "Commit changes to template (modify)" },
    "template_export"   : { "func" : "template_export",
                            "help" : "Export template record" },
    "template_checkout" : { "func" : "template_checkout",
                            "help" : "Checkout a template" },
    "template_instantiate": { "func" : "template_instantiate",
                            "help" : "Instantiate a template" },
    "template_swapin":    { "func" : "template_swapin",
                            "help" : "Swapin a preloaded template instance" },
    "template_swapout"  : { "func" : "template_swapout",
                            "help" : "Terminate template instance" },
    "template_startrun" : { "func" : "template_startrun",
                            "help" : "Start new experiment run" },
    "template_modrun"   : { "func" : "template_modrun",
                            "help" : "Modify resources for run" },
    "template_stoprun"  : { "func" : "template_stoprun",
                            "help" : "Stop current experiment run" },
};

#
# Process a single command line
#
def do_method(module, method, params):
    # This is parsed by the Proxy object.
    URI = "ssh://" + login_id + "@" + xmlrpc_server + "/xmlrpc";
    # Get a handle on the server,
    server = SSHServerProxy(URI) 

    # Get a pointer to the function we want to invoke.
    methodname = module + "." + method
    meth = getattr(server, methodname)

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
        
    return rval, response

#
# node_reboot
#
class reboot:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "wcfe:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        module = "node";
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
                pass
            elif opt == "-c":
                params["reconfig"] = "yes";
                pass
            elif opt == "-f":
                params["power"] = "yes";
                pass
            elif opt == "-w":
                params["wait"] = "yes";
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                module = "experiment";
                pass
            pass

        if module == "node":
            # Do this after so --help is seen.
            if len(req_args) < 1:
                self.usage();
                return -1;

            params["nodes"]  = string.join(req_args, ",");
            pass
        else:
            if len(req_args):
                self.usage();
                return -1;
            pass
        
        rval,response = do_method(module, "reboot", params);
        return rval;

    def usage(self):
        print "node_reboot [options] node [node ...]";
        print "node_reboot [options] -e pid,eid"
	print "where:";
	print "    -w    - Wait for nodes is come back up";
	print "    -c    - Reconfigure nodes instead of rebooting";
	print "    -f    - Power cycle nodes (skip reboot!)";
 	print "    -e    - Reboot all nodes in an experiment";
	print "  node    - Node to reboot (pcXXX)";
        return
    pass

#
# node_list
#
class node_list:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "pPvhHme:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        which  = "phys";
        params = {};
        params["aspect"] = "mapping";

        for opt, val in opts:
            if opt == "--help":
                self.usage()
                return 0
                pass
            elif opt == "-p":
                which = "phys";
                pass
            elif opt == "-P":
                which = "phystype";
                pass
            elif opt == "-v":
                which = "virt";
                pass
            elif opt == "-h":
                which = "pphys";
                pass
            elif opt == "-H":
                which = "pphysauxtype";
                pass
            elif opt == "-m":
                which = "mapping";
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        # Do this after so --help is seen.
        if len(req_args) or not params.has_key("proj"):
            self.usage();
            return -1;

        rval,response = do_method("experiment", "info", params);
        if rval:
            return rval;

        for node in response["value"]:
            val = response["value"][node];

            if which == "virt":
                print node, " ",
                pass
            elif which == "phys":
                print val["node"], " ",
                pass
            elif which == "phystype":
                print ("%s=%s") % (val["node"],val["type"]), " ",
                pass
            elif which == "pphys":
                print val["pnode"], " ",
                pass
            elif which == "pphysauxtype":
                print "%s=%s"  % (val["pnode"],val["auxtype"]), " ",
                pass
            elif which == "mapping":
                print "%s=%s" % (node, val["pnode"]),
                pass
            pass
        print "";
        
        return rval;

    def usage(self):
	print "node_list [options] -e pid,eid";
	print "where:";
	print "     -p   - Print physical (Emulab database) names (default)";
	print "     -P   - Like -p, but include node type";
	print "     -v   - Print virtual (experiment assigned) names";
	print "     -h   - Print physical name of host for virtual nodes";
	print "     -H   - Like -h, but include node auxtypes";
 	print "     -e   - Project and Experiment ID to list";
        return
    pass


#
# node_avail
#
class node_avail:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        params = {}
        
        try:
            opts, req_args = getopt.getopt(self.argv, "hp:t:c:", [
                "help", "project=", "node-type=", "node-class=" ])

            for opt, val in opts:
                if opt in ("-h", "--help"):
                    self.usage();
                    return 0
                elif opt in ("-p", "--project"):
                    params["proj"] = val
                    pass
                elif opt in ("-c", "--node-class"):
                    params["class"] = val
                    pass
                elif opt in ("-t", "--node-type"):
                    params["type"] = val
                    pass
                pass
            pass
        except  getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        rval,response = do_method("node", "available", params)
        return rval

    def usage(self):
        print "node_avail [-p project] [-c class] [-t type]"
        print "Print the number of available nodes."
        print "where:"
        print "     -p project  - Specify project credentials for node types"
        print "                   that are restricted"
        print "     -c class    - The node class (Default: pc)"
        print "     -t type     - The node type"
        print
        print "example:"
        print "  $ node_avail -t pc850"
        return

    pass


#
# delay_config
#
class delay_config:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "s:me:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        params = {};
        for opt, val in opts:
            if opt == "--help":
                self.usage()
                return 0
                pass
            elif opt == "-m":
                params["persist"] = "yes";
                pass
            elif opt == "-s":
                params["src"] = val;
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) < 2:
                self.usage();
                return -1;
            
            params["proj"]  = req_args[0];
            params["exp"]   = req_args[1];
            req_args        = req_args[2:];
            pass

        # Do this after so --help is seen.
        if len(req_args) < 2:
            self.usage();
            return -1;

        # Next should be the link we want to control
        params["link"]  = req_args[0];

        # Now we turn the rest of the arguments into a dictionary
        linkparams = {}
        for linkparam in req_args[1:]:
            plist = string.split(linkparam, "=", 1)
            if len(plist) != 2:
                print ("Parameter, '" + linkparam
                       + "', is not of the form: param=value!")
                self.usage();
                return -1
            
            linkparams[plist[0]] = plist[1];
            pass
        params["params"] = linkparams;

        rval,response = do_method("experiment", "delay_config", params);
        return rval;

    def usage(self):
	print "delay_config [options] -e pid,eid link PARAM=value ...";
	print "delay_config [options] pid eid link PARAM=value ...";
	print "where:";
	print "     -m   - Modify virtual experiment as well as current state";
	print "     -s   - Select the source of the link to change";
 	print "     -e   - Project and Experiment ID to operate on";
        print "   link   - Name of link from your NS file (ie: 'link1')";
        print "";
        print "PARAMS";
        print " BANDWIDTH=NNN   - N=bandwidth (10-100000 Kbits per second)";
        print " PLR=NNN         - N=lossrate (0 <= plr < 1)";
        print " DELAY=NNN       - N=delay (one-way delay in milliseconds > 0)";
        print " LIMIT=NNN       - The queue size in bytes or packets";
        print " QUEUE-IN-BYTES=N- 0 means in packets, 1 means in bytes";
        print "RED/GRED Options: (only if link was specified as RED/GRED)";
        print " MAXTHRESH=NNN   - Maximum threshold for the average Q size";
        print " THRESH=NNN      - Minimum threshold for the average Q size";
        print " LINTERM=NNN     - Packet dropping probability";
        print " Q_WEIGHT=NNN    - For calculating the average queue size\n";
        return

#
# wilink_config
#
class wilink_config:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "s:me:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        params = {};
        for opt, val in opts:
            if opt == "--help":
                self.usage()
                return 0
                pass
            elif opt == "-m":
                params["persist"] = "yes";
                pass
            elif opt == "-s":
                params["src"] = val;
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) < 2:
                self.usage();
                return -1;
            
            params["proj"]  = req_args[0];
            params["exp"]   = req_args[1];
            req_args        = req_args[2:];
            pass

        # Do this after so --help is seen.
        if len(req_args) < 2:
            self.usage();
            return -1;

        # Next should be the link we want to control
        params["link"]  = req_args[0];

        # Now we turn the rest of the arguments into a dictionary
        linkparams = {}
        for linkparam in req_args[1:]:
            plist = string.split(linkparam, "=", 1)
            if len(plist) != 2:
                print ("Parameter, '" + param
                       + "', is not of the form: param=value!")
                self.usage();
                return -1
            
            linkparams[plist[0]] = plist[1];
            pass
        params["params"] = linkparams;

        rval,response = do_method("experiment", "link_config", params);
        return rval;

    def usage(self):
	print "wilink_config [options] -e pid,eid link PARAM=value ...";
	print "wilink_config [options] pid eid link PARAM=value ...";
	print "where:";
	print "     -m   - Modify virtual experiment as well as current state";
	print "     -s   - Select the source of the link to change";
 	print "     -e   - Project and Experiment ID to operate on";
        print "   link   - Name of link from your NS file (ie: 'link1')";
        print "";
        print "Special Param";
        print " ENABLE=yes/no   - Bring the link up or down (or ENABLE=up/down)";
        print ""
        print "*********************** WARNING *******************************"
        print "   wilink_config is used to configure WIRELESS INTERFACES!"
        print "   Use delay_config to change the traffic shaping parameters"
        print "   on normal links and lans."
        print "***************************************************************"
        return

#
# savelogs
#
class savelogs:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "e:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        params = {};
        for opt, val in opts:
            if opt == "--help":
                self.usage()
                return 0
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        # Do this after so --help is seen.
        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) < 2:
                self.usage();
                return -1;
            
            params["proj"]  = req_args[0];
            params["exp"]   = req_args[1];
            pass

        rval,response = do_method("experiment", "savelogs", params);
        return rval;

    def usage(self):
	print "savelogs -e pid,eid";
	print "savelogs pid eid";
	print "where:";
 	print "     -e   - Project and Experiment ID";
        return
    pass

#
# portstats
#
class portstats:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "azqcpe", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        params = {};
        for opt, val in opts:
            if opt == "--help":
                self.usage()
                return 0
                pass
            elif opt == "-e":
                params["errors-only"] = "yes";
                pass
            elif opt == "-a":
                params["all"] = "yes";
                pass
            elif opt == "-z":
                params["clear"] = "yes";
                pass
            elif opt == "-q":
                params["quiet"] = "yes";
                pass
            elif opt == "-c":
                params["absolute"] = "yes";
                pass
            elif opt == "-p":
                params["physnames"] = "yes";
                pass
            pass

        # Do this after so --help is seen.
        if not params.has_key("physnames"):
            if len(req_args) < 2:
                self.usage();
                return -1;
            
            params["proj"]  = req_args[0];
            params["exp"]   = req_args[1];
            req_args        = req_args[2:];
            pass
        elif len(req_args) < 1:
            self.usage();
            return -1;
        
        # Send the rest of the args along as a list.
        params["nodeports"] = req_args;

        rval,response = do_method("experiment", "portstats", params);
        return rval;

    def usage(self):
 	print "portstats <-p | pid eid> [vname ...] [vname:port ...]";
	print "where:";
        print "    -e    - Show only error counters";
        print "    -a    - Show all stats";
        print "    -z    - Zero out counts for selected counters after printing";
        print "    -q    - Quiet: don't actually print counts - useful with -z";
        print "    -c    - Print absolute, rather than relative, counts";
        print "    -p    - The machines given are physical, not virtual, node";
        print "            IDs. No pid and eid should be given with this option";
        print "";
        print "If only pid and eid are given, prints out information about all";
        print "ports in the experiment. Otherwise, output is limited to the";
        print "nodes and/or ports given.";
        print "";
        print "NOTE: Statistics are reported from the switch's perspective.";
        print "      This means that 'In' packets are those sent FROM the node,";
        print "      and 'Out' packets are those sent TO the node.";
        print "";
        print "In the output, packets described as 'NUnicast' or 'NUcast' are ";
        print "non-unicast (broadcast or multicast) packets.";
        return
    pass


#
# eventsys_control
#
class eventsys_control:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "e:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        params = {};
        for opt, val in opts:
            if opt == "--help":
                self.usage()
                return 0
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) < 2:
                self.usage();
                return -1;
            
            params["proj"]   = req_args[0];
            params["exp"]    = req_args[1];
            params["action"] = req_args[2];
            pass
        elif len(req_args) != 1:
            self.usage();
            return -1;
        else:
            params["action"] = req_args[0];            

        rval,response = do_method("experiment", "eventsys_control", params);
        return rval;

    def usage(self):
	print "eventsys_control -e pid,eid start|stop|replay";
	print "eventsys_control pid eid start|stop|replay";
	print "where:";
 	print "     -e   - Project and Experiment ID";
        print "   stop   - Stop the event scheduler";
        print "  start   - Start the event stream from time index 0";
        print " replay   - Replay the event stream from time index 0";
        return
    pass

#
# nscheck
#
class nscheck:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            pass

        # Do this after so --help is seen.
        if len(req_args) != 1:
            self.usage();
            return -1;

        try:
            nsfilestr = open(req_args[0]).read();
            pass
        except:
            print "Could not open file: " + req_args[0];
            return -1;

        params["nsfilestr"] = nsfilestr;
        rval,response = do_method("experiment", "nscheck", params);
        return rval;

    def usage(self):
	print "nscheck nsfile";
	print "where:";
	print " nsfile    - Path to NS file you to wish check for parse errors";
        return
    pass

#
# startexp
#
class startexp:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv,
                                           "iwfqS:L:a:l:E:g:p:e:n:",
                                           [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-i":
                params["batch"] = "no";
                pass
            elif opt == "-E":
                params["description"] = val;
                pass
            elif opt == "-g":
                params["group"] = val;
                pass
            elif opt == "-e":
                params["exp"] = val;
                pass
            elif opt == "-p":
                params["proj"] = val;
                pass
            elif opt == "-S":
                params["swappable"]     = "no";
                params["noswap_reason"] = val;
                pass
            elif opt == "-L":
                params["idleswap"]          = 0;
                params["noidleswap_reason"] = val;
                pass
            elif opt == "-l":
                params["idleswap"] = val;
                pass
            elif opt == "-a":
                params["max_duration"] = val;
                pass
            elif opt == "-f":
                params["noswapin"] = "yes"
                pass
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-n":
                params["nsfilestr"] = val
                pass
            pass

        # Do this after so --help is seen.
        if not params.has_key("nsfilestr") and len(req_args) != 1:
            self.usage();
            return -1;

        if not params.has_key("nsfilestr"):
            try:
                nsfilestr = open(req_args[0]).read();
                pass
            except:
                print "Could not open file: " + req_args[0];
                return -1;
            params["nsfilestr"] = nsfilestr;

        rval,response = do_method("experiment", "startexp", params);
        return rval;

    def usage(self):
        print "startexp [-q] [-i [-w]] [-f] [-E description] [-g gid]";
        print "         [-S reason] [-L reason] [-a <time>] [-l <time>]";
        print "         -p <pid> -e <eid> <nsfile>";
	print "where:";
        print "   -i   - swapin immediately; by default experiment is batched";
        print "   -w   - wait for non-batchmode experiment to preload or swapin";
        print "   -f   - preload experiment (do not swapin or queue yet)";
        print "   -q   - be less chatty";
        print "   -S   - Experiment cannot be swapped; must provide reason";
        print "   -L   - Experiment cannot be IDLE swapped; must provide reason";
        print "   -a   - Auto swapout NN minutes after experiment is swapped in";
        print "   -l   - Auto swapout NN minutes after experiment goes idle";
        print "   -E   - A pithy sentence describing your experiment";
        print "   -g   - The subgroup in which to create the experiment";
        print "   -p   - The project in which to create the experiment";
        print "   -e   - The experiment name (unique, alphanumeric, no blanks)";
        print "nsfile  - NS file to parse for experiment";
        return
    pass

#
# swapexp
#
class swapexp:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "we:s:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-s":
                params["direction"] = val
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) < 2:
                self.usage();
                return -1;
            
            params["proj"] = req_args[0];
            params["exp"]  = req_args[1];
            req_args       = req_args[2:];
            pass

        if not params.has_key("direction") and len(req_args) != 1:
            self.usage();
            return -1;
        else:
            params["direction"] = req_args[0];
            pass

        rval,response = do_method("experiment", "swapexp", params);
        return rval;

    def usage(self):
	print "swapexp -e pid,eid in|out";
	print "swapexp pid eid in|out";
	print "where:";
	print "     -w   - Wait for experiment to finish swapping";
 	print "     -e   - Project and Experiment ID";
        print "     in   - Swap experiment in  (must currently be swapped out)";
        print "    out   - Swap experiment out (must currently be swapped in)";
        print ""
        print "By default, swapexp runs in the background, sending you email ";
        print "when the transition has completed. Use the -w option to wait";
        print "in the foreground, returning exit status. Email is still sent.";
        return
    pass

#
# modexp
#
class modexp:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "rswe:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-r":
                params["reboot"] = "yes"
                pass
            elif opt == "-s":
                params["restart_eventsys"] = "yes"
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) != 3:
                self.usage();
                return -1;
            
            params["proj"] = req_args[0];
            params["exp"]  = req_args[1];
            req_args       = req_args[2:];
            pass
        elif len(req_args) != 1:
            self.usage();
            return -1;

        #
        # Read in the NS file to pass along.
        # 
        try:
            nsfilestr = open(req_args[0]).read();
            pass
        except:
            print "Could not open file: " + req_args[0];
            return -1;

        params["nsfilestr"] = nsfilestr;
        rval,response = do_method("experiment", "modify", params);
        return rval;

    def usage(self):
	print "modexp [-r] [-s] [-w] -e pid,eid nsfile";
	print "modexp [-r] [-s] [-w] pid eid nsfile";
	print "where:";
	print "     -w   - Wait for experiment to finish swapping";
 	print "     -e   - Project and Experiment ID";
        print "     -r   - Reboot nodes (when experiment is active)";
        print "     -s   - Restart event scheduler (when experiment is active)";
        print ""
        print "By default, modexp runs in the background, sending you email ";
        print "when the transition has completed. Use the -w option to wait";
        print "in the foreground, returning exit status. Email is still sent.";
        print ""
        print "The experiment can be either swapped in *or* swapped out.";
        print "If the experiment is swapped out, the new NS file replaces the ";
        print "existing NS file (the virtual topology is updated). If the";
        print "experiment is swapped in (active), the physical topology is";
        print "also updated, subject to the -r and -s options above";
        return
    pass

#
# endexp
#
class endexp:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "hwe:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) != 2:
                self.usage();
                return -1;
            
            params["proj"] = req_args[0];
            params["exp"]  = req_args[1];
            pass
        elif len(req_args) != 0:
            self.usage();
            return -1;

        rval,response = do_method("experiment", "endexp", params);
        return rval;

    def usage(self):
	print "endexp [-w] -e pid,eid";
	print "endexp [-w] pid eid";
	print "where:";
	print "     -w   - Wait for experiment to finish terminating";
 	print "     -e   - Project and Experiment ID";
        print ""
        print "By default, endexp runs in the background, sending you email ";
        print "when the transition has completed. Use the -w option to wait";
        print "in the foreground, returning exit status. Email is still sent.";
        print ""
        print "The experiment can be terminated when it is currently swapped";
        print "in *or* swapped out.";
        return
    pass

#
# expinfo
#
class expinfo:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "nmldae:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        show   = [];
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-n":
                show.append("nodeinfo");
                pass
            elif opt == "-m":
                show.append("mapping");
                pass
            elif opt == "-l":
                show.append("linkinfo");
                pass
            elif opt == "-d":
                show.append("shaping");
                pass
            elif opt == "-a":
                show = ["nodeinfo", "mapping", "linkinfo", "shaping"];
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) != 2:
                self.usage();
                return -1;
            
            params["proj"] = req_args[0];
            params["exp"]  = req_args[1];
            pass
        elif len(req_args) or len(show) == 0:
            self.usage();
            return -1;

        params["show"] = string.join(show, ",");
        rval,response = do_method("experiment", "expinfo", params);
        return rval;

    def usage(self):
	print "expinfo [-n] [-m] [-l] [-d] [-a] -e pid,eid";
	print "expinfo [-n] [-m] [-l] [-d] [-a] pid eid";
	print "where:";
 	print "     -e   - Project and Experiment ID";
        print "     -n   - Show node info";
        print "     -m   - Show node mapping";
        print "     -l   - Show link info";
        print "     -a   - Show all of the above";
        return
    pass

class tbuisp:
    def __init__(self, argv=None):
        self.argv = argv
        return

    def apply(self):
        params = {}
  
        try:
            opts, req_args = getopt.getopt(self.argv, "h", [
                "help", ])

            for opt, val in opts:
                if opt in ("-h", "--help"):
                    self.usage();
                    return 0
                pass

            if len(req_args) < 3:
                raise getopt.error(
                    "error: a file and one or more nodes must be specified")
            pass
        except  getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

	params["op"] = req_args[0]
        file = req_args[1]
        params["nodes"] = req_args[2:]
  
        try:
            filestr = open(file).read()
            pass
        except:
            print "Could not open file: " + file
            return -1

        # I don't know what is in the file, so we'll base64 encode before
        # sending it over XML-RPC.  On the receiving side you'll get a Binary
        # object with the bits stored in the data field.  In code:
        #
        #   filestr = params["filestr"].data
        #
        # I think...
        params["filestr"] = xmlrpclib.Binary(filestr)

        rval,response = do_method("node", "tbuisp", params)
                
        return rval
                    
    def usage(self):
        print "tbuisp <operation> <file> <node1> [<node2> ...]"
        print "where:"
	print "     operation - Operation to perform. Currently, only 'upload'"
	print "                 is supported"
        print "     file      - Name of the to upload to the mote"
        print "     nodeN     - The physical node name ..."
        print
        print "example:"   
        print "  $ tbuisp upload foo.srec mote1"
        return
        
    pass

#
# expwait
#
class expwait:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "ht:e:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        show   = [];
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-t":
                params["timeout"] = val;
                pass
            elif opt == "-e":
                pid,eid = string.split(val, ",")
                params["proj"] = pid;
                params["exp"]  = eid;
                pass
            pass

        #
        # The point is to allow backwards compatable pid eid arguments, but
        # move to the better -e pid,eid format eventually. 
        #
        if not params.has_key("proj"):
            if len(req_args) != 3:
                self.usage();
                return -1;
            
            params["proj"]  = req_args[0];
            params["exp"]   = req_args[1];
            params["state"] = req_args[2];
            pass
        elif len(req_args) != 1:
            self.usage();
            return -1;
        else:
            params["state"] = req_args[0];
            pass

        rval,response = do_method("experiment", "statewait", params);
        return rval;

    def usage(self):
	print "expwait [-t timeout] -e pid,eid state";
	print "expwait [-t timeout] pid eid state";
	print "where:";
 	print "     -e   - Project and Experiment ID";
        print "     -t   - Maximum time to wait (in seconds).";
        return
    pass

#
# tipacl (get console acl goo)
#
class tipacl:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        params = {};
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
                pass
            pass

        # Do this after so --help is seen.
        if len(req_args) != 1:
            self.usage();
            return -1;
        
        params["node"]  = req_args[0];

        rval,response = do_method("node", "console", params);

        if rval == 0:
            for key in response["value"]:
                val = response["value"][key];
                print "%s: %s" % (key, val)
                pass
            pass
        
        return rval;

    def usage(self):
	print "tipacl node";
	print "where:";
	print "  node    - Node to get tipacl data for (pcXXX)";
        return
    pass

#
# template_commit
#
class template_commit:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv,
                                           "we:p:E:t:r:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        guid   = None
        params = {}
        eid    = None
        pid    = None
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-e":
                params["exp"] = val
                eid = val
                pass
            elif opt == "-p":
                params["proj"] = val
                pid = val;
                pass
            elif opt == "-E":
                params["description"] = val;
                pass
            elif opt == "-t":
                params["tid"] = val;
                pass
            elif opt == "-r":
                params["tag"] = val;
                pass
            pass
        
        # Try to infer the template guid/vers from the current path.
        if len(req_args) == 1:
            params["guid"] = req_args[0]
            pass
        elif pid == None and eid == None:
            (guid, subdir) = infer_template()
            if subdir:
                params["path"] = os.getcwd()
                pass
            else:
                self.usage();
                return -1
            pass
        else:
            self.usage();
            return -1
        
        print "Committing template; please be (very) patient!"
        
        rval,response = do_method("template", "template_commit", params);
        return rval;
    
    def usage(self):
	print "template_commit [-w]"
	print "template_commit [-w] [-e eid | -r tag] <guid/vers>"
        print "template_commit [-w] -p pid -e eid"
	print "where:";
	print "     -w     - Wait for template to finish commit";
        print "     -e     - Commit from specific template instance (eid)"
        print "     -E     - A pithy sentence describing your experiment"
        print "     -t     - The template name (alphanumeric, no blanks)"
        print "     -p     - Project for -e option (pid)"
        print "    guid    - Template GUID"
        print ""
        print "By default, commit runs in the background, sending you email ";
        print "when the operation has completed. Use the -w option to wait";
        print "in the foreground, returning exit status. Email is still sent.";
        print ""
        print "Environment:"
        print "  cwd   The template will be inferred from the current"
        print "        working directory if it is inside a template checkout."
        return
    pass

class template_export:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "i:r:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        instance = None
        params   = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-i":
                params["instance"] = val
                instance = val
                pass
            elif opt == "-r":
                params["run"] = val
                pass
            pass
        
        # Try to infer the template guid/vers from the current path.
        if instance == None:
            self.usage();
            return -1
        
        rval,response = do_method("template", "export", params);
        return rval;
    
    def usage(self):
	print "template_export [-r runidx] -i instance_id";
	print "where:";
        print "     -i   - Export specific template instance (idx)";
        print "     -r   - Optional run index to export";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory, if it is inside the templates's"
        print "        directory (e.g. /proj/foo/templates/10005/18)."
        return
    pass

class template_checkout:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "d", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        if (len(req_args) == 0):
            self.usage()
            return -1

        params         = {}
        params["guid"] = req_args[0]
        params["path"] = os.getcwd()
        
        rval,response = do_method("template", "checkout", params);
        return rval;
    
    def usage(self):
	print "template_checkout guid/vers";
        print ""
        print "Environment:"
        print "  cwd   The template checkout is placed in the current dir"
        return
    pass

#
# template_instantiate
#
class template_instantiate:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv,
                                           "bwqS:L:a:l:E:e:x:",
                                           [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        guid        = None
        xmlfilename = None
        params      = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-b":
                params["batch"] = "yes";
                pass
            elif opt == "-E":
                params["description"] = val;
                pass
            elif opt == "-e":
                params["exp"] = val;
                pass
            elif opt == "-x":
                xmlfilename = val;
                pass
            elif opt == "-S":
                params["swappable"]     = "no";
                params["noswap_reason"] = val;
                pass
            elif opt == "-L":
                params["idleswap"]          = 0;
                params["noidleswap_reason"] = val;
                pass
            elif opt == "-l":
                params["idleswap"] = val;
                pass
            elif opt == "-a":
                params["max_duration"] = val;
                pass
            elif opt == "-f":
                params["noswapin"] = "yes"
                pass
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-p":
                params["preload"] = "yes"
                pass
            pass

        # Try to infer the template guid/vers from the current path.
        if len(req_args) == 0:
            guid = infer_guid(os.getcwd())
            if guid == None:
                (guid, subdir) = infer_template()
                if guid == None:
                    self.usage()
                    return -1
                pass
            pass
        elif len(req_args) == 1:
            guid = req_args[0]
            pass
        else:
            self.usage();
            return -1
        
        params["guid"] = guid

        if xmlfilename:
            try:
                params["xmlfilestr"] = open(xmlfilename).read();
                pass
            except:
                print "Could not open file: " + req_args[0];
                return -1;
            pass
        
        rval,response = do_method("template", "instantiate", params);
        return rval;

    def usage(self):
        print "template_instantiate [-q] [-E description] [-p xmlfilename] ";
        print "     [-b] [-p] [-S reason] [-L reason] [-a <time>] [-l <time>]";
        print "     -e <eid> <guid>";
	print "where:";
        print "   -b   - queue for the batch system";
        print "   -p   - preload only; do not swapin";
        print "   -w   - wait for non-batchmode experiment to instantiate";
        print "   -q   - be less chatty";
        print "   -S   - Instance cannot be swapped; must provide reason";
        print "   -L   - Instance cannot be IDLE swapped; must provide reason";
        print "   -a   - Auto swapout NN minutes after instance is swapped in";
        print "   -l   - Auto swapout NN minutes after instance goes idle";
        print "   -E   - A pithy sentence describing your experiment";
        print "   -x   - XML file of parameter bindings";
        print "   -e   - The instance name (unique, alphanumeric, no blanks)";
        print "   guid - Template GUID";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory if it is inside a template checkout."
        return
    pass

#
# template_swapin
#
class template_swapin:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "wqe:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;
        
        guid   = None
        params = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-q":
                params["quiet"] = "yes"
                pass
            elif opt == "-e":
                params["exp"]  = val;
                pass
            pass

        # Try to infer the template guid/vers from the current path.
        if len(req_args) == 0:
            guid = infer_guid(os.getcwd())
            if guid == None:
                self.usage();
                return -1
            pass
        elif len(req_args) == 1:
            guid = req_args[0]
        else:
            self.usage();
            return -1
            
        params["guid"] = guid

        rval,response = do_method("template", "swapin", params);
        return rval;

    def usage(self):
	print "template_swapin -e id [<guid/vers>]";
	print "where:";
        print "     -e   - Instance ID (aka eid)";
        print "     -q   - be less chatty";
	print "     -w   - Wait for instance to finish swapping in";
        print "    guid  - Template GUID";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory, if it is inside the templates's"
        print "        directory (e.g. /proj/foo/templates/10005/18)."
        return
    pass

#
# template_swapout
#
class template_swapout:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "we:", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        guid   = None
        params = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-e":
                params["exp"]  = val;
                pass
            pass

        # Try to infer the template guid/vers from the current path.
        if len(req_args) == 0:
            guid = infer_guid(os.getcwd())
            if guid == None:
                self.usage();
                return -1
            pass
        elif len(req_args) == 1:
            guid = req_args[0]
        else:
            self.usage();
            return -1
            
        params["guid"] = guid

        rval,response = do_method("template", "swapout", params);
        return rval;

    def usage(self):
	print "template_swapout -e id [<guid/vers>]";
	print "where:";
        print "     -e   - Instance ID (aka eid)";
	print "     -w   - Wait for instance to finish terminating";
        print "    guid  - Template GUID";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory, if it is inside the templates's"
        print "        directory (e.g. /proj/foo/templates/10005/18)."
        return
    pass

#
# template_startrun and template_modrun
#
class template_startrun:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "we:E:r:p:cx:my:",
                                           [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        pid         = None
        guid        = None
        xmlfilename = None
        params      = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-E":
                params["description"] = val;
                pass
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-m":
                params["modify"] = "yes"
                pass
            elif opt == "-c":
                params["clear"] = "yes"
                pass
            elif opt == "-r":
                params["runid"] = val
                pass
            elif opt == "-y":
                params["params"] = val
                pass
            elif opt == "-p":
                pid = val
                pass
            elif opt == "-x":
                xmlfilename = val;
                pass
            elif opt == "-e":
                params["exp"]  = val;
                pass
            pass

        # Try to infer the template guid/vers from the current path.
        if pid == None:
            if len(req_args) == 0:
                guid = infer_guid(os.getcwd())
                if guid == None:
                    self.usage();
                    return -1
                pass
            elif len(req_args) == 1:
                guid = req_args[0]
                pass
            else:
                self.usage();
                return -1
            
            params["guid"] = guid
            pass
        else:
            params["pid"] = pid
            pass
        
        if xmlfilename:
            try:
                params["xmlfilestr"] = open(xmlfilename).read();
                pass
            except:
                print "Could not open file: " + req_args[0];
                return -1;
            pass
        
        rval,response = do_method("template", "startrun", params);
        return rval;

    def usage(self):
	print "template_startrun [-r <id>] [-E <descr>] -e id [-p <pid> | <guid/vers>]";
	print "where:";
        print "     -E   - A pithy sentence describing your run";
        print "     -x   - XML file of parameter bindings";
        print "     -y   - Default params, one of template,instance,lastrun";
        print "     -r   - A token (id) for the run";
        print "     -e   - Instance ID (aka eid)";
	print "     -w   - Wait until run has started";
	print "     -m   - Reparse ns file (effectively a swap modify)";
        print "     -c   - run loghole clean before starting run";
        print "    guid  - Template GUID";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory, if it is inside the templates's"
        print "        directory (e.g. /proj/foo/templates/10005/18)."
        return
    pass

class template_modrun:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "we:E:r:p:cx:my:",
                                           [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        pid         = None
        guid        = None
        xmlfilename = None
        params      = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-E":
                params["description"] = val;
                pass
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-m":
                params["modify"] = "yes"
                pass
            elif opt == "-c":
                params["clear"] = "yes"
                pass
            elif opt == "-r":
                params["runid"] = val
                pass
            elif opt == "-y":
                params["params"] = val
                pass
            elif opt == "-p":
                pid = val
                pass
            elif opt == "-x":
                xmlfilename = val;
                pass
            elif opt == "-e":
                params["exp"]  = val;
                pass
            pass

        # Try to infer the template guid/vers from the current path.
        if pid == None:
            if len(req_args) == 0:
                guid = infer_guid(os.getcwd())
                if guid == None:
                    self.usage();
                    return -1
                pass
            elif len(req_args) == 1:
                guid = req_args[0]
                pass
            else:
                self.usage();
                return -1
            
            params["guid"] = guid
            pass
        else:
            params["pid"] = pid
            pass
        
        if xmlfilename:
            try:
                params["xmlfilestr"] = open(xmlfilename).read();
                pass
            except:
                print "Could not open file: " + req_args[0];
                return -1;
            pass
        
        rval,response = do_method("template", "modrun", params);
        return rval;

    def usage(self):
	print "template_modrun [-r <id>] [-E <descr>] -e id [-p <pid> | <guid/vers>]";
	print "where:";
        print "     -E   - A pithy sentence describing your run";
        print "     -x   - XML file of parameter bindings";
        print "     -y   - Default params, one of template,instance,lastrun";
        print "     -r   - A token (id) for the run";
        print "     -e   - Instance ID (aka eid)";
	print "     -w   - Wait until run has started";
	print "     -m   - Reparse ns file (effectively a swap modify)";
        print "     -c   - run loghole clean before starting run";
        print "    guid  - Template GUID";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory, if it is inside the templates's"
        print "        directory (e.g. /proj/foo/templates/10005/18)."
        return
    pass

#
# template_stoprun
#
class template_stoprun:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            opts, req_args = getopt.getopt(self.argv, "we:qp:t:i", [ "help" ]);
            pass
        except getopt.error, e:
            print e.args[0]
            self.usage();
            return -1;

        pid         = None
        guid        = None
        params      = {}
        
        for opt, val in opts:
            if opt in ("-h", "--help"):
                self.usage()
                return 0
            elif opt == "-w":
                params["wait"] = "yes"
                pass
            elif opt == "-q":
                params["quiet"] = "yes"
                pass
            elif opt == "-p":
                pid = val
                pass
            elif opt == "-t":
                params["token"] = val;
                pass
            elif opt == "-i":
                params["ignoreerrors"] = "yes";
                pass
            elif opt == "-e":
                params["exp"] = val;
                pass
            pass

        # Try to infer the template guid/vers from the current path.
        if pid == None:
            if len(req_args) == 0:
                guid = infer_guid(os.getcwd())
                if guid == None:
                    self.usage();
                    return -1
                pass
            elif len(req_args) == 1:
                guid = req_args[0]
                pass
            else:
                self.usage();
                return -1
            params["guid"] = guid
            pass
        else:
            params["pid"] = pid
            pass

        rval,response = do_method("template", "stoprun", params);
        return rval;

    def usage(self):
	print "template_stoprun [-w] -e id [-p <pid> | <guid/vers>]";
	print "where:";
        print "     -e   - Instance ID (aka eid)";
        print "     -i   - Ignore errors and force Run to stop";
	print "     -w   - Wait for Run to finish stopping";
        print "    guid  - Template GUID";
        print ""
        print "Environment:"
        print "  cwd   The template GUID will be inferred from the current"
        print "        working directory, if it is inside the templates's"
        print "        directory (e.g. /proj/foo/templates/10005/18)."
        return
    pass

#
# Infer template guid from path
#
def infer_guid(path):
    guid = None
    vers = None
    dirs = path.split(os.path.sep)
    if ((len(dirs) < 6) or
        (not (("proj" in dirs) and ("templates" in dirs))) or
        (len(dirs) < (dirs.index("templates") + 2))):
        return None
    else:
        guid = dirs[dirs.index("templates") + 1]
        vers = dirs[dirs.index("templates") + 2]
        pass
    return guid + "/" + vers

#
# Different version, that crawls up tree looking for .template file.
# Open up file and get the guid/vers, but also return path of final directory.
#
def infer_template():
    rootino = os.stat("/").st_ino
    cwd     = os.getcwd()
    guid    = None
    vers    = None
    subdir  = None

    try:
        while True:
            if os.access(".template", os.R_OK):
                fp = open(".template")
                line = fp.readline()
                while line:
                    m = re.search('^GUID:\s*([\w]*)\/([\d]*)$', line)
                    if m:
                        guid    = m.group(1)
                        vers    = m.group(2)
                        subdir  = os.getcwd()
                        fp.close();
                        return (guid + "/" + vers, subdir)
                    line = fp.readline()
                    pass
                fp.close();
                break
            if os.stat(".").st_ino == rootino:
                break
            os.chdir("..")
            pass
        pass
    except:
        pass
        
    os.chdir(cwd)
    return (guid, subdir)    
    pass

#
# Infer pid and eid
#
def infer_pideid(path):
    pid = None
    eid = None
    dirs = path.split(os.path.sep)
    if ((len(dirs) < 6) or
        (not (("proj" in dirs) and ("exp" in dirs))) or
        (len(dirs) < (dirs.index("exp") + 1))):
        return (None, None)
    else:
        pid = dirs[dirs.index("proj") + 1]
        eid = dirs[dirs.index("proj") + 3]
        pass
    return (pid, eid)


def run(cmd, argv=None):
    if API.has_key(cmd):
        handler = API[cmd]["func"];
        instance = eval(handler + "(argv=command_argv)");
        return instance.apply();
    return -1
