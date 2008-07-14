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
import re
import sys
import socket
import os
import popen2
import getopt
import string

# Maximum size of an NS file that the server will accept. 
MAXNSFILESIZE = (1024 * 512)

#
# This class defines a simple structure to return back to the caller.
# It includes a basic response code (success, failure, badargs, etc),
# as well as a return "value" which can be any valid datatype that can
# be represented in XML (int, string, hash, float, etc). You can also
# send back some output (a string with embedded newlines) to print out
# to the user.
#
# Note that XMLRPC does not actually return a "class" to the caller; It gets
# converted to a hashed array (Python Dictionary), but using a class gives
# us a ready made constructor.
#
# WARNING: If you change this stuff, also change libxmlrpc.pm in this dir.
#
RESPONSE_SUCCESS        = 0
RESPONSE_BADARGS        = 1
RESPONSE_ERROR          = 2
RESPONSE_FORBIDDEN      = 3
RESPONSE_BADVERSION     = 4
RESPONSE_SERVERERROR    = 5
RESPONSE_TOOBIG         = 6
RESPONSE_REFUSED        = 7  # Emulab is down, try again later.
RESPONSE_TIMEDOUT       = 8

class EmulabResponse:
    def __init__(self, code, value=0, output=""):
        self.code     = code            # A RESPONSE code
        self.value    = value           # A return value; any valid XML type.
        self.output   = re.sub(         # Pithy output to print
            r'[^' + re.escape(string.printable) + ']', "", output)
        
        return

#
# Read an nsfile and return a single string. 
#
def readnsfile(nsfilename, debug):
    nsfilestr  = ""
    try:
        fp = os.open(nsfilename, os.O_RDONLY)

        while True:
	    str = os.read(fp, 1024)

            if not str:
                break
            nsfilestr = nsfilestr + str
            pass

        os.close(fp)

    except:
        if debug:
            print "%s:%s" % (sys.exc_type, sys.exc_value)
            pass

        print "batchexp: Cannot read NS file '" + nsfilename + "'"
        return None
        pass

    return nsfilestr


