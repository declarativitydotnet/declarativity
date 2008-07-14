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
# Given script runs aggregates.olg test and checks the test output
#
# Assumptions -  Three nodes with ports 20202, 20203, 20204. Execute the following:
#  
#	* At 20202:
#	* tests/runOverLog -DME=\"localhost:20202\" -DNEIGHBOR1=\"localhost:20203\" -DNEIGHBOR2=\"localhost:20204\" -o unitTests/olg/aggregates.olg -n localhost -p 20202
#
#	* At 20203:
#	* tests/runOverLog -DME=\"localhost:20203\" -DNEIGHBOR1=\"localhost:20202\" -DNEIGHBOR2=\"localhost:20204\" -o unitTests/olg/aggregates.olg -n localhost -p 20203
#
# 	* At 20204:
#	* tests/runOverLog -DME=\"localhost:20204\" -DNEIGHBOR1=\"localhost:20202\" -DNEIGHBOR2=\"localhost:20203\" -o unitTests/olg/aggregates.olg -n localhost -p 20204
#
# Expected output - (multiple tuples may be present in the output and the order of the results can vary)
#
#	* At 20202:
#	##Print[RecvEvent!smallestNeighborOf!d2_eca!localhost:20202]:  [smallestNeighborOf(localhost:20202, localhost:20203)]
#	##Print[RecvEvent!smallestNeighborOf!d2_eca!localhost:20202]:  [smallestNeighborOf(localhost:20202, localhost:20204)]
#
#	* At 20203
#
#	##Print[RecvEvent!smallestNeighborOf!d2_eca!localhost:20203]:  [smallestNeighborOf(localhost:20203, localhost:20202)]
#	##Print[RecvEvent!largestNeighborOf!d1_eca!localhost:20203]:  [largestNeighborOf(localhost:20203, localhost:20204)]
#
#	*At 20204
#	##Print[RecvEvent!largestNeighborOf!d1_eca!localhost:20204]:  [largestNeighborOf(localhost:20204, localhost:20203)]
#	##Print[RecvEvent!largestNeighborOf!d1_eca!localhost:20204]:  [largestNeighborOf(localhost:20204, localhost:20202)]
#
#
####################################

#!/usr/bin/python
import os
import time
import threading
import re
import sys
import getopt
import subprocess

# Usage function
def usage():
        print """
                aggregates.py -E <planner path> -B <olg path> -T <time in seconds>

                -E              planner path
                -B              olg path
	        -T              time (secs) for test to run
                -h              prints usage message
        """

# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout_20202, stdout_20203, stdout_20204):
	output = ""
        for line in stdout_20202.readlines():
                output = output + line
	p = re.compile(r"""
		([#][#]Print\[RecvEvent \s* RULE \s* d2\]: \s*  \[smallestNeighborOf\(localhost:20202, \s* localhost:20203\)\]
 		.*
		[#][#]Print\[RecvEvent \s* RULE \s* d2\]: \s* \[smallestNeighborOf\(localhost:20202, \s* localhost:20204\)\]
		|
		[#][#]Print\[RecvEvent \s* RULE \s* d2\]: \s* \[smallestNeighborOf\(localhost:20202, \s* localhost:20204\)\]
		.*
		[#][#]Print\[RecvEvent \s* RULE \s* d2\]: \s*  \[smallestNeighborOf\(localhost:20202, \s* localhost:20203\)\])
		""", re.VERBOSE|re.DOTALL)

	flag = p.search(output)
	
	if flag == 0:
		print "Test failed"
		return


	output = ""
        for line in stdout_20203.readlines():
                output = output + line
        p = re.compile(r"""
		([#][#]Print\[RecvEvent \s* RULE \s* d2\]: \s* \[smallestNeighborOf\(localhost:20203, \s* localhost:20202\)\]
		.*
		[#][#]Print\[RecvEvent \s* RULE \s* d1\]: \s* \[largestNeighborOf\(localhost:20203, \s* localhost:20204\)\]
		|
		[#][#]Print\[RecvEvent \s* RULE \s* d1\]: \s* \[largestNeighborOf\(localhost:20203, \s* localhost:20204\)\]
		.*
		[#][#]Print\[RecvEvent \s* RULE \s* d2\]: \s* \[smallestNeighborOf\(localhost:20203, \s* localhost:20202\)\])
                """, re.VERBOSE|re.DOTALL)

        flag = p.search(output)

	if flag == 0:
                print "Test failed"
                return


	output = ""
        for line in stdout_20204.readlines():
                output = output + line
        p = re.compile(r"""
		([#][#]Print\[RecvEvent \s* RULE \s* d1\]: \s* \[largestNeighborOf\(localhost:20204, \s* localhost:20203\)\]
		.*
		[#][#]Print\[RecvEvent \s* RULE \s* d1\]: \s* \[largestNeighborOf\(localhost:20204, \s* localhost:20202\)\]
		|
		[#][#]Print\[RecvEvent \s* RULE \s* d1\]: \s* \[largestNeighborOf\(localhost:20204, \s* localhost:20202\)\]
		.*
		[#][#]Print\[RecvEvent \s* RULE \s* d1\]: \s* \[largestNeighborOf\(localhost:20204, \s* localhost:20203\)\])	
                """, re.VERBOSE|re.DOTALL)

	flag = p.search(output)
	
	if flag == 0:
                print "Test failed"
                return
        else:
                print "Test passed"
	

#Function to kill the child after a set time
def kill_pid(stdout_20202, stdout_20203, stdout_20204, pid_20202, pid_20203, pid_20204):
        #print "killing child"
        os.kill(pid_20202, 3)
	os.kill(pid_20203, 3)
	os.kill(pid_20204, 3)
	#print "program killed"
	script_output(stdout_20202, stdout_20203, stdout_20204)


opt, arg = getopt.getopt(sys.argv[1:], 'B:E:T:h')

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
        args=[executable_path , '-DME=\"localhost:20202\"', '-DNEIGHBOR1=\"localhost:20203\"', '-DNEIGHBOR2=\"localhost:20204\"', '-o', os.path.join(olg_path,'aggregates.olg'), '-n', 'localhost', '-p', '20202', '2>&1']
        p_20202 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
time.sleep(1)
pid_20202 = p_20202.pid
#print pid_20202

try:
        args=[executable_path , '-DME=\"localhost:20203\"', '-DNEIGHBOR1=\"localhost:20202\"', '-DNEIGHBOR2=\"localhost:20204\"', '-o', os.path.join(olg_path,'aggregates.olg'), '-n', 'localhost', '-p', '20203', '2>&1']
	p_20203 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
time.sleep(1)
pid_20203 = p_20203.pid
#print pid_20203

try:
        args=[executable_path , '-DME=\"localhost:20204\"', '-DNEIGHBOR1=\"localhost:20202\"', '-DNEIGHBOR2=\"localhost:20203\"', '-o', os.path.join(olg_path,'aggregates.olg'), '-n', 'localhost', '-p', '20204', '2>&1']
	p_20204 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
pid_20204 = p_20204.pid
#print pid_20204


if os.getpid() != pid_20202 and os.getpid() != pid_20203 and os.getpid() != pid_20204:
	t = threading.Timer(int(time_interval), kill_pid, [p_20202.stdout, p_20203.stdout, p_20204.stdout, pid_20202, pid_20203, pid_20204])
        t.start()
