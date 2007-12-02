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
# Given script runs localCounter.olg test and checks the test output
#
# Assumption - program is running at localhost:10000
#
#Expected output - (should get 100 tuples of the type given below where i increments from 1 to 100 with step 1)
#	##Print[AddAction!counter!g1_eca!localhost:10000]:  [counter(localhost:10000, i)]
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
                localCounter.py -E <planner path> -B <olg path>

                -E              planner path
                -B              olg path
                -h              prints usage message
        """

# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
        output = ""
	i = 1
	result = 1
        for line in stdout.readlines():
		p = re.compile(r"""
			(^[#][#]Print\[AddAction!counter!g1_eca!localhost:10000\]: \s* 
			\[counter\(localhost:10000, \s* """ + 
			str(i) + 
			"""\)\])
			""", re.VERBOSE|re.MULTILINE)
	
		flag = p.match(line)
		if flag:
			i = i+1
			#print "Test passed" 
		else:
			#print "Test failed"
			result = 0
			break

	if result == 0 and i <= 100:
		print "Test failed"
	elif i > 101 or i < 101:
		print "Test failed"
	else:
		print "Test passed"

#Function to kill the child after a set time
def kill_pid(stdout, pid):
        #print "killing child"
        os.kill(pid, 3)
        #print "program killed"
        script_output(stdout)


opt, arg = getopt.getopt(sys.argv[1:], 'B:E:h')

for key,val in opt:
        if key=='-B':
                olg_path = val
        elif key == '-E':
                executable_path = val
        elif key == '-h':
                usage()
                sys.exit(0)
try:
        args=[executable_path , '-o', olg_path +'/localCounter.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
