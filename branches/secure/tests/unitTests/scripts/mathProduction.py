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
# Given script runs mathProduction.olg test and checks the test output
#
# Assumption - program is running at localhost:10000
#
#Expected output - (the order of the tuples can vary)
#	##Print[RecvEvent!result!result_watchStub!localhost:10000]:  [result(localhost:10000, 23)]
#	##Print[RecvEvent!inclusion!inclusion_watchStub!localhost:10000]:  [inclusion(localhost:10000)]
#	##Print[RecvEvent!constantInclusion!constantInclusion_watchStub!localhost:10000]:  [constantInclusion(localhost:10000)]
#	##Print[RecvEvent!notInclusion!notInclusion_watchStub!localhost:10000]:  [notInclusion(localhost:10000)]
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
                mathProduction.py -E <planner path> -B <olg path>

                -E              planner path
                -B              olg path
                -h              prints usage message
        """

# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
       	lines=[]
        for line in stdout.readlines():
		lines.append(line.rstrip())
	
	lines.sort()
	i =1
	for line in lines:
		if i == 1:
			p = re.compile(r"""
				(^[#][#]Print\[RecvEvent!constantInclusion!constantInclusion_watchStub!localhost:10000\]: \s* \[constantInclusion\(localhost:10000\)\])
                        	""", re.VERBOSE)
		elif i == 2:
			p = re.compile(r"""
				(^[#][#]Print\[RecvEvent!inclusion!inclusion_watchStub!localhost:10000\]: \s* \[inclusion\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 3:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!notConstantInclusion!notConstantInclusion_watchStub!localhost:10000\]: \s* \[notConstantInclusion\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 4:
			p = re.compile(r"""
				(^[#][#]Print\[RecvEvent!notInclusion!notInclusion_watchStub!localhost:10000\]: \s* \[notInclusion\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 5:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!result!result_watchStub!localhost:10000\]: \s* \[result\(localhost:10000, \s* 23\)\])
                                """, re.VERBOSE)
		else:
			print "Test failed"
			break
	
		flag = p.match(line)
        	if flag:
        		i = i+1
       	 	else:
                	result = 0
                	break
	
	if i>6 or i<6:
		print "Test failed"
	else:
		print "Test passed"
		

#Function to kill the child after a set time
def kill_pid(stdout, pid):
        #print "killing child"
        os.kill(pid, 3)
        #print "program killed"
        script_output(stdout)


opt, arg = getopt.getopt(sys.argv[1:], 'B:E:T:h')

for key,val in opt:
        if key=='-B':
                olg_path = val
        elif key == '-E':
                executable_path = val
        elif key == '-h':
                usage()
                sys.exit(0)
try:
        args=[executable_path , '-o', olg_path +'/mathProduction.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
