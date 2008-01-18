#!/usr/bin/env python

import sys
import getopt
import os

sys.path.append(os.path.join(os.getcwd(), "lib")) # NOTE: don't changeme!
import emulab
import mkns2

DEFAULT_EXPRARGS = ["-i", "-E", "Run experiement script", "-p", "P2"]

##
# Print the usage statement to stdout.
#
def usage():
    print "Run experiment on Emulab."
    print ("Usage: " + sys.argv[0]
                     + " [-h] [-l login] [-s script] [-r p2-rpm-path] name "
                     + "(-n nsfile | routers hosts)")
    print
    print "Options:"
    print "  -h, --help \t  Display this help message"
    print "  -l, --login\t  Set the login id (defaults to $USER)"
    print "  -p, --prog \t  A program/script that each node should run upon startup"
    print "  -r, --rpm  \t  The path to the p2 rpm"
    print "  -n, --ns   \t  The path to a ns file"
    print
    print "Required arguments:"
    print "  name   \t  The experiment name (unique, alphanumeric, no blanks)"
    print "  routers\t  The number of routers"
    print "  hosts  \t  The number of hosts"
    print
    print "Example:"
    print ("  "
           + sys.argv[0]
           + " -s ./run.sh -r /proj/P2/rpms/p2-fc6-update.rpm 4 20")
    return


#
# Process program arguments.
# 
try:
    # Parse the options,
    opts, req_args =  getopt.getopt(sys.argv[1:],
                      "hl:p:r:n:",
                      [ "help", "login", "prog", "rpm", "nsfile"])
    # ... act on them appropriately, and
    nsfile = None
    script = None
    rpm = None
    for opt, val in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
            pass
        elif opt in ("-l", "--login"):
            login_id = val
            pass
        elif opt in ("-p", "--prog"):
            script = val
            pass
        elif opt in ("-r", "--rpm"):
            rpm = val
            pass
        elif opt in ("-n", "--nsfile"):
            nsfile = val
            pass
        pass
    pass
except getopt.error, e:
    print e.args[0]
    usage()
    sys.exit(2)
    pass

if len(req_args) < 1:
    usage()
    sys.exit(2)
    pass

ns = None
if nsfile:
    f = open(ns, 'r')
    ns = f.read()
    f.close()
    print ns
elif len(req_args) != 3:
    usage()
    sys.exit(2)
    pass
else:
    ns2argv = []
    if rpm:
        ns2argv.append("-r")
        ns2argv.append(rpm)
    if script:
        ns2argv.append("-p")
        ns2argv.append(script)

    ns2argv.append(req_args[1]) # nrouters
    ns2argv.append(req_args[2]) # nhosts
   
    myns = mkns2.ns2(argv=ns2argv)
    ns = myns.apply()


if not ns: 
    print "ERROR: No nsfile given or generated!"
    sys.exit(2)
    pass

exprargs = DEFAULT_EXPRARGS
exprargs.append("-e")
exprargs.append(req_args[0])
exprargs.append("-n")
exprargs.append(ns)

expr = emulab.startexp(exprargs)
code = expr.apply()
sys.exit(code)

