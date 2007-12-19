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
# Given script runs unboundHeadVariables.olg test and checks the test output
#
# Assumption - program is runing at localhost:10000
#
# Expected output -
#	##COMPILE ERROR: program commandLine, node localhost:10000, error code 0: ERROR Rule: q1
#        Unknown variable: Variable(localhost:10000, C)
#        SCHEMA: (Location(localhost:10000, NI), Variable(localhost:10000, A), Variable(localhost:10000, B))
#
#
####################################

#!/usr/bin/python
import os
import time
import threading
import re
import getopt
import subprocess
import sys

# Usage function
def usage():
        print """
                unboundHeadVariables.py -E <planner path> -B <olg path> -T <time in seconds>

                -E              planner path
                -B              olg path
		-T              time (secs) for test to run
                -h              prints usage message
        """


# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
        output = ""
        for line in stdout.readlines():
                output = output + line
	
	p = re.compile(r"""
		([#][#] COMPILE \s* ERROR: \s* program \s* commandLine, \s* node \s* localhost:10000, \s*
		error \s* code \s* 0: \s* ERROR \s* Rule: \s* q1 \s*
	        Unknown \s* variable: \s* Variable\(localhost:10000, \s* C\) \s*
	        SCHEMA: \s* \(Location\(localhost:10000, \s* NI\), \s* Variable\(localhost:10000, \s* A\), \s* 
		Variable\(localhost:10000, \s* B\)\))
		""", re.VERBOSE)

	flag = p.search(output)
	if flag:
		print "Test passed"
		#print flag.group()
	else:
		print "Test failed"
		print "Port 10000 output:"
		print output

#Function to kill the child after a set time
def kill_pid(stdout, pid):
        #print "killing child"
        os.kill(pid, 3)
        #print "program killed"
        script_output(stdout)


try:
        opt, arg = getopt.getopt(sys.argv[1:], 'B:E:T:h')
except getopt.GetoptError:
        usage()
        sys.exit(1)

if len(opt) != 3:
        usage()
        sys.exit(1)

for key,val in opt:
        if key=='-B':
                olg_path = val
        elif key == '-E':
                executable_path = val
	elif key == '-T':
                time_interval = val
        elif key == '-h':
                usage()
                sys.exit(0)
try:
        args=[executable_path , '-o', os.path.join(olg_path, 'unboundHeadVariables.olg'), '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(int(time_interval), kill_pid, [p.stdout, p.pid])
        t.start()
