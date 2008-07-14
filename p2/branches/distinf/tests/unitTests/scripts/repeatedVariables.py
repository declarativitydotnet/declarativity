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
# Given script runs repeatedVariables.olg test and checks the test output
#
# Assumption - program is runing at localhost:10000
#
# Expected output - (there should be three such tuples)
#	##Print[SendAction: RULE r2]:  [out(localhost:10000, 1, 1, 1)]
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
                repeatedVariables.py -E <planner path> -B <olg path> -T <time in seconds>

                -E              planner path
                -B              olg path
		-T              time (secs) for test to run
                -h              prints usage message
        """


# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
        output = ""
	whole_output = ""
	lines = []
        for line in stdout.readlines():
                #print line
                whole_output = whole_output + line
                p = re.compile('^[#][#]Print.*$',re.VERBOSE|re.DOTALL)
                if(p.match(line)):
                        lines.append(line.rstrip().lstrip())
        i = 1
	result = 1
	for line in lines:
        	p = re.compile(r"""
			(^[#][#] Print\[SendAction: \s* RULE \s* r2\]: \s*  
			\[out\(localhost:10000, \s* 1, \s* 1, \s* 1\)\])
                        """, re.VERBOSE)        
		
		flag = p.match(line)
                if flag:
                	i = i + 1
                else:
                        result = 0
                        break

	if i>4 or i<4 or result == 0:
		print "Test failed"
                print "Port 10000 output:"
                print whole_output
	else:
		print "Test passed"


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
        args=[executable_path , '-o', olg_path +'/repeatedVariables.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(int(time_interval), kill_pid, [p.stdout, p.pid])
        t.start()
