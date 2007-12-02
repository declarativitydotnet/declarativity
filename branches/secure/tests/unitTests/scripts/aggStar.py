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
# Given script runs aggStar.olg test and checks the test output
#
# Assumption - program is running at localhost:10000
#
#Expected output - (the order of hte tuples can vary and E is a random number)
#	##Print[SendAction!prodEvent1!q1_eca!localhost:10000]:  [prodEvent1(localhost:10000, 4)]
#	##Print[SendAction!prodEvent2!q2_eca!localhost:10000]:  [prodEvent2(4, localhost:10000)]
#	##Print[SendAction!prodEvent3!q3_eca!localhost:10000]:  [prodEvent3(4, E, localhost:10000)]
#	##Print[SendAction!prodEvent1Empty!q1empty_eca!localhost:10000]:  [prodEvent1Empty(localhost:10000, 0)]
#	##Print[SendAction!prodEvent2Empty!q2empty_eca!localhost:10000]:  [prodEvent2Empty(0, localhost:10000)]
#	##Print[SendAction!prodEvent3Empty!q3empty_eca!localhost:10000]:  [prodEvent3Empty(0, E, localhost:10000)]
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
                aggStar.py -E <planner path> -B <olg path>

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
				(^[#][#]Print\[SendAction!prodEvent1!q1_eca!localhost:10000\]: \s* \[prodEvent1\(localhost:10000, \s* 4\)\])
                        	""", re.VERBOSE)
		elif i == 2:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction!prodEvent1Empty!q1empty_eca!localhost:10000\]: \s* \[prodEvent1Empty\(localhost:10000, \s* 0\)\])
                                """, re.VERBOSE)
		elif i == 3:
			p = re.compile(r"""
                                (^[#][#]Print\[SendAction!prodEvent2!q2_eca!localhost:10000\]: \s* \[prodEvent2\(4, \s* localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 4:
			p = re.compile(r"""
                                (^[#][#]Print\[SendAction!prodEvent2Empty!q2empty_eca!localhost:10000\]: \s* \[prodEvent2Empty\(0, \s* localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 5:
			p = re.compile(r"""
                                (^[#][#]Print\[SendAction!prodEvent3!q3_eca!localhost:10000\]: \s* \[prodEvent3\(4, \s* [0-9]+, \s* localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 6:
			p = re.compile(r"""
                                (^[#][#]Print\[SendAction!prodEvent3Empty!q3empty_eca!localhost:10000\]: \s* \[prodEvent3Empty\(0, \s* [0-9]+, \s* localhost:10000\)\])
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
	
	if i >7 or i <7:
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
        args=[executable_path , '-o', olg_path + '/aggStar.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
