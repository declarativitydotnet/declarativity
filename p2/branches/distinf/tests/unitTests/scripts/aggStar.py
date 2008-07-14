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
#Expected output - (the order of the tuples can vary and E is a random number)
#	##Print[SendAction: RULE q1]:  [prodEvent1(localhost:10000, 4)]
#	##Print[SendAction: RULE q2]:  [prodEvent2(4, localhost:10000)]
#	##Print[SendAction: RULE q3]:  [prodEvent3(4, E, localhost:10000)]
#	##Print[SendAction: RULE q1empty]:  [prodEvent1Empty(localhost:10000, 0)]
#	##Print[SendAction: RULE q2empty]:  [prodEvent2Empty(0, localhost:10000)]
#	##Print[SendAction: RULE q3empty]:  [prodEvent3Empty(0, E, localhost:10000)]

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
                aggStar.py -E <planner path> -B <unitTest olg dir path> -T <time in seconds>

                -E              planner path
                -B              unitTest olg dir path
		-T		time (secs) for test to run
                -h              prints usage message
        """

# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
       	lines=[]
	whole_output = ""
        for line in stdout.readlines():
		whole_output = whole_output + line
		p = re.compile('^[#][#]Print.*$',re.VERBOSE|re.DOTALL) 
		if(p.match(line)):
			lines.append(line.rstrip())
	
	lines.sort()
	i = 1
	result = 1
	for line in lines:
		if i == 1:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction: \s* RULE \s* q1\]: \s* \[prodEvent1\(localhost:10000, \s* 4\)\])
                        	""", re.VERBOSE)
		elif i == 2:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction: \s* RULE \s* q1empty\]: \s* \[prodEvent1Empty\(localhost:10000, \s* 0\)\])
                                """, re.VERBOSE)
		elif i == 3:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction: \s* RULE \s* q2\]: \s* \[prodEvent2\(4, \s* localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 4:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction: \s* RULE \s* q2empty\]: \s* \[prodEvent2Empty\(0, \s* localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 5:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction: \s* RULE \s* q3\]: \s* \[prodEvent3\(4, \s* [0-9]+, \s* localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 6:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction: \s* RULE \s* q3empty\]: \s* \[prodEvent3Empty\(0, \s* [0-9]+, \s* localhost:10000\)\])
				""", re.VERBOSE)
		else:
			result = 0
			break
	
		flag = p.match(line)
        	if flag:
        		i = i+1
       	 	else:
                	result = 0
                        break
	
	if result == 0 or i>7 or i<7:
		print "Test failed"
		print "Port 10000 output"
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
        args=[executable_path , '-o', os.path.join(olg_path, 'aggStar.olg'), '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(int(time_interval), kill_pid, [p.stdout, p.pid])
        t.start()
