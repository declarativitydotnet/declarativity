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
# Given script runs chord.olg on Emulab
#

####################################

#!/usr/bin/python
import os
import signal
import time
import threading
import re
import getopt
import subprocess
import sys
import socket
import GUID

LANDMARK_NODEID = "node0000"
PORT = "10000"

# Usage function
def usage():
    print """
    chord.py [-h] [-E <planner-path=/usr/local/p2/bin/runStagedOverlog>] [-B <chord.olg dir path=/usr/local/p2/unitTests/olg/chord.olg>] [-T <time in seconds=600>]

    -E        planner path
    -B        chord.olg dir path
    -T        time (secs) for test to run
    -h        prints usage message
    """

# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
    for line in stdout.readlines():
        p = re.compile(r"""
            ^[#][#]Print\[InsertEvent: \s* RULE \s* .*\]: \s* \[bestSucc\(
                        \s* (.*),  # Source node address
                        \s* (.*),  # Source nodeid
                        \s* (.*)   # Destination node address
                        \)\]
                           """, re.VERBOSE)
        if p.match(line):
            bestSucc = [x for x in p.split(line) if x]
            print "MATCH: ", bestSucc

#Function to kill the child after a set time
def kill_pid(stdout, pid):
    #print "killing child"
    os.kill(pid, signal.SIG_DFL)
    #print "program killed"
    script_output(stdout)

try:
    opt, arg = getopt.getopt(sys.argv[1:], 'B:E:T:h')
except getopt.GetoptError:
    usage()
    sys.exit(1)
flags = {"olg_path" : "/usr/local/p2/unitTests/chord.olg", 
         "exe_path" : "/usr/local/p2/bin/runStagedOverlog",
         "time"     : 600}

for key,val in opt:
    if key=='-B':
        flags["olg_path"] = val
    elif key == '-E':
        flags["exe_path"] = val
    elif key == '-T':
        flags["time"] = int(val)
    elif key == '-h':
        usage()
        sys.exit(0)

try:
    hostname = socket.gethostname().split('.')[0]
    args=[flags["exe_path"], 
          '-o', flags["olg_path"], 
          '-D', "LANDMARK=\"" + LANDMARK_NODEID + ":" + PORT + "\"",
          '-D', "LOCALADDRESS=\"" + hostname + ":" + PORT + "\"",
          '-n', hostname,
          '-p', PORT,
          '-D', "NODEID=0x" + GUID.generate() + "I",
          '2>&1']
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
    print "Execution failed"
    print e
    sys.exit(0)


if os.getpid() != p.pid:
    t = threading.Timer(flags["time"], kill_pid, [p.stdout, p.pid])
    t.start()
    sys.exit(0)
