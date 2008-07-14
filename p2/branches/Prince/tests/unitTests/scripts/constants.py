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
# Given script runs constants.olg test and checks the test output
#
# Assumption - program is running at localhost:11111
# 
#       tests/runStagedOverlog -o unitTests/olg/constants.olg -DLOCALID=\"localhost:11111\" -p 11111
#
# Expected output - (the order of the results can vary)
#
#	##Print[HeadProjection: RULE r2]:  [eventJoinConstant(localhost:11111)]
#	##Print[HeadProjection: RULE r2a]:  [eventJoin(localhost:11111)]
#	##Print[HeadProjection: RULE r4]:  [tableTableJoin(localhost:11111)]
#	##Print[HeadProjection: RULE r5]:  [newTableOldTableConstantJoin(localhost:11111)]
#	##Print[HeadProjection: RULE r5]:  [bothTablesConstantJoin(localhost:11111)]
#	##Print[HeadProjection: RULE r6]:  [newTableConstantOldTableJoin(localhost:11111)]
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
                aggStar.py -E <planner path> -B <unitTest olg dir path> -T <time in seconds>

                -E              planner path
                -B              unitTest olg dir path
		-T              time (secs) for test to run
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
	for line in lines:
		if i == 1:
			p = re.compile(r"""
                        	(^[#][#]Print\[HeadProjection: \s* RULE \s* r2\]: \s* \[eventJoinConstant\(localhost:11111\)\])
				""", re.VERBOSE)
		elif i == 2:
			p = re.compile(r"""
				(^[#][#]Print\[HeadProjection: \s* RULE \s* r2a\]: \s* \[eventJoin\(localhost:11111\)\])
                                """, re.VERBOSE)
		elif i == 3:
			p = re.compile(r"""
                                (^[#][#]Print\[HeadProjection: \s* RULE \s* r4\]: \s* \[tableTableJoin\(localhost:11111\)\])
				""", re.VERBOSE)
		elif i == 4:
			p = re.compile(r"""
				(^[#][#]Print\[HeadProjection: \s* RULE \s* r5\]: \s* \[bothTablesConstantJoin\(localhost:11111\)\])
                                """, re.VERBOSE)
		elif i == 5:
			p = re.compile(r"""
				(^[#][#]Print\[HeadProjection: \s* RULE \s* r5\]: \s* \[newTableOldTableConstantJoin\(localhost:11111\)\])
                                """, re.VERBOSE)
		elif i == 6:
			p = re.compile(r"""
				(^[#][#]Print\[HeadProjection: \s* RULE \s* r6\]: \s* \[newTableConstantOldTableJoin\(localhost:11111\)\])
				""", re.VERBOSE)
		else:
			break
	
		flag = p.match(line)
        	if flag:
        		i = i+1
       	 	else:
                	result = 0
                	break
	
	if i >7 or i <7:
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
        args=[executable_path , '-o', os.path.join(olg_path,'constants.olg'), '-DLOCALID=\"localhost:11111"', '-p', '11111', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(int(time_interval), kill_pid, [p.stdout, p.pid])
        t.start()
