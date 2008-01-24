#
# * This file is distributed under the terms in the attached LICENSE file.
# * If you do not find this file, copies can be found by writing to:
# * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# * Or
# * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776,
# * Berkeley, CA,  94707. Attention: P2 Group.
#
########### Description ###########
#
#
###################################
#!/usr/bin/env python

import sys
import getopt
import os
import time
import threading
import re
import subprocess
import sys
import shutil

# The default arguments for running an expriment.
# See emulab.py class startexp for details regarding these
# default arguments.
DEFAULT_EXPRARGS = ["-i", "-w", "-E", "Run experiement script", "-p", "P2"]

# Default login ID taken from environment
login_id = os.environ["USER"]

##
# Print the usage statement to stdout.
#
def usage():
    print "Run experiment on Emulab."
    print ("Usage: " + sys.argv[0]
                     + " [-h] [[-b] [-B] [-S svn-dir]] [-l login] [-p prog] [-r rpm-path] "
                     + "expr-name (-n nsfile | routers hosts)")
    print
    print "Options:"
    print "  -h, --help       \t  Display this help message"
    print "  -b, --build      \t  Build before running experiment"
    print "  -B  --build-only \t  Build only"
    print "  -S  --svn-dir    \t  P2 SVN directory (defaults to svn=https://svn.declarativity.net/trunk)"
    print "  -l, --login      \t  Set the login id (defaults to $USER)"
    print "  -p, --prog       \t  A program/script that each node should run upon startup"
    print "  -r, --rpm-path   \t  The path to the p2 rpm"
    print "  -n, --ns         \t  A ns file to use to setup the experiment"
    print
    print "Required arguments for experiements:"
    print "  name   \t  The experiment name (unique, alphanumeric, no blanks)"
    print "  routers\t  The number of routers"
    print "  hosts  \t  The number of hosts"
    print
    print "Example:"
    print ("  "
           + sys.argv[0]
           + " -s ./run.sh -r /proj/P2/rpms/p2-fc6-update.rpm 4 20")
    return

# This function builds the P2 rpm. It will checkout P2 from the
# main trunk, and compile that code base on an Emulab node with
# OS = FC6-UPDATE.
def build_rpm(proj, svn):
    args=["svn", "co", svn, "p2"]         # SVN checkout of p2 trunk
    p = subprocess.Popen(args)
    exitcode = p.wait()
    args=["tar", "-cvf", "p2.tar", "p2"]  # Tar the p2 directory just checkout
    p = subprocess.Popen(args)
    p.wait()
    args=["gzip", "-f", "p2.tar"]         # Gzip the p2 tar file
    p = subprocess.Popen(args)
    p.wait()
    shutil.rmtree("./p2")                 # Clean up checkout directory

    # Copy gzip file of p2 over to emulab.
    args=["scp", "p2.tar.gz", login_id + "@users.emulab.net:"]
    p = subprocess.Popen(args)
    p.wait()

    # Copy myself over to emulab
    args=["scp", sys.argv[0], login_id + "@users.emulab.net:"]
    p = subprocess.Popen(args)
    p.wait()

    # Create an experiement using the build.ns file, which 
    # should be a single node with OS = FC6-UPDATE
    experiment("build", "build.ns") # NOTE: Blocks until node is swapped in.

    # Execute myself on the just created Emulab node with argument '-m', which
    # simply calls the 'do_rpm' method. 
    # ASSUMES: node name in build.ns is 'node.build.P2.emulab.net'
    args=["ssh", login_id + "@node.build." + proj + ".emulab.net", "python", sys.argv[0], "-m"]
    p = subprocess.Popen(args)
    p.wait()

    # No longer need build experiement since 'do_rpm' copies newly created
    # rpm to the /proj/P2/rpms directory. 
    #endexpr(proj, "build") 

# This method is called when given the '-m' arguement. The '-m' arguement
# is not listed in the help menu because this routine should only 
# be called on Emulab via an instance of this program on the local node.
# That is this method should be called from 'build_rpm' only.
def do_rpm(rpm):
    # Install 'cmake' on the Emulab node
    args=["sudo", "yum", "-y", "install", "cmake"]
    p = subprocess.Popen(args)
    exitcode = p.wait()

    # Untar the p2 tar file to the /tmp directory
    args=["tar", "-xvf", "p2.tar.gz", "-C", "/tmp"]
    p = subprocess.Popen(args)
    exitcode = p.wait()

    # Ensure we have a build directory under p2.
    if os.access("/tmp/p2/build", os.F_OK) == False:
        args=["mkdir", "/tmp/p2/build"]
        p = subprocess.Popen(args)
        exitcode = p.wait()
    os.chdir("/tmp/p2/build")      # Go to build directory
    args=["cmake", ".."]           # Execute cmake
    p = subprocess.Popen(args)
    exitcode = p.wait()
    args=["make", "p2_rpm"]       # make the rpm
    p = subprocess.Popen(args)
    exitcode = p.wait()
 
    # Move the newly created RPM file to the 
    # project RPM directory e.g., /proj/P2/rpms/p2-fc6-update.rpm
    args=["sudo", "mv", "/tmp/p2/build/RPM/RPMS/i386/p2-1-1.i386.rpm", rpm]
    p = subprocess.Popen(args)
    exitcode = p.wait()

# Terminate an experiment, does not block.
def endexpr(project, name):
    exprargs = [project, name]
    expr = emulab.endexp(exprargs)
    code = expr.apply()

# Create an experiement. Will block until all nodes have been swapped in.
def experiment(expr_name, nsfile, nrouters=None, nhosts=None, rpm=None, script=None):
    if nsfile:
        # We have an nsfile, it will be used to start the new experiement.
        f = open(nsfile)
        ns = f.read()
        f.close()
    else:
        # No nsfile so we need to generate one using Brent's stuff.
        ns2argv = []
        if rpm:
            # Make sure all nodes install the rpm.
            ns2argv.append("-r")
            ns2argv.append(rpm)
        if script:
            # Make sure all nodes run the script at startup.
            ns2argv.append("-p")
            ns2argv.append(script)
    
        ns2argv.append(nrouters) # nrouters
        ns2argv.append(nhosts)   # nhosts
       
        # Call into Brent's code to create an NS description
        myns = mkns2.ns2(argv=ns2argv)
        ns = myns.apply()

    if not ns: 
        print "ERROR: No nsfile given or generated!"
        sys.exit(2)
        pass
    
    exprargs = DEFAULT_EXPRARGS
    exprargs.append("-e")
    exprargs.append(expr_name) # Experiement name
    exprargs.append("-n")
    exprargs.append(ns)        # String of the NS file
    
    expr = emulab.startexp(exprargs)
    code = expr.apply()

#
# Process program arguments.
# 
try:
    # Parse the options,
    opts, req_args =  getopt.getopt(sys.argv[1:],
                      "hbBml:p:r:n:",
                      [ "help", "build", "build-only", "make-rpm", "login", "prog", "rpm", "nsfile"])
    # ... act on them appropriately, and
    flags = {"svn-dir"  : "https://svn.declarativity.net/trunk",
             "rpm-dir"  : "/proj/P2/rpms",
             "make-rpm" : None,
             "nsfile"   : None,
             "prog"     : None,
             "rpm-file" : "p2-fc6-update.rpm",
             "proj"    : "P2"}
    build      = None
    build_only = None
    for opt, val in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
            pass
        elif opt in ("-l", "--login"):
            login_id = val
            pass
        elif opt in ("-p", "--prog"):
            flags["prog"] = val
            pass
        elif opt in ("-r", "--rpm-path"):
            flags["rpm-file"] = val
            pass
        elif opt in ("-m", "--make-rpm"):
            flags["make-rpm"] = True
            pass
        elif opt in ("-b", "--build"):
            build = True
            pass
        elif opt in ("-B", "--build-only"):
            build_only = True
            pass
        elif opt in ("-S", "--svn-dir"):
            flags["svn-dir"] = val
            pass
        elif opt in ("-n", "--nsfile"):
            flags["nsfile"] = val
            pass
        pass
    pass
except getopt.error, e:
    print e.args[0]
    usage()
    sys.exit(2)
    pass

if not flags["make-rpm"] and not build_only and len(req_args) < 1:
    usage()
    sys.exit(2)
    pass

if not flags["make-rpm"]:
    sys.path.append(os.path.join(os.getcwd(), "lib")) # NOTE: don't changeme!
    import emulab
    import mkns2

if flags["make-rpm"] == True:
    do_rpm(os.path.join(flags["rpm-dir"], flags["rpm-file"]))
elif build_only == True:
    build_rpm(flags["proj"], flags["svn-dir"])
else:
    if build: build_rpm(flags["proj"], flags["svn-dir"])

    if flags["nsfile"]:
        experiment(req_args[0], flags["nsfile"], None, None, 
                   os.path.join(flags["rpm-dir"], flags["rpm-file"]), flags["prog"])
    elif len(req_args) == 3:
        experiment(req_args[0], None, req_args[1], req_args[2], 
                   os.path.join(flags["rpm-dir"], flags["rpm-file"]), flags["prog"])
    else: 
        usage()
        sys.exit(0)
   
