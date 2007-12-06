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
# Given script runs locSpec.olg test and checks the test output
#
# Assumptions -  Two nodes running at port 11111 and 22222. Execute the following:
#  
#	* At 11111:
#	* tests/runOverLog -o unitTests/olg/locSpec.olg -DLOCALID=\"localhost:11111\" -p 11111 -DNEIGHBORID=\"localhost:22222\" -DEVENTID=235691
#
#	* At 22222:
#	* tests/runOverLog -o unitTests/olg/locSpec.olg -DLOCALID=\"localhost:22222\" -p 22222 -DNEIGHBORID=\"localhost:11111\" -DEVENTID=235691
#
#
# Expected output - 
#
#	* At 11111:
#	* ##Print[AddAction!link!r5ECAMat!localhost:11111]:  [link(localhost:22222, localhost:11111, 235691, secondLocSpec)]
#
#	* At 22222:
#	* ##Print[AddAction!link!r3_eca!localhost:22222]:  [link(localhost:22222, localhost:11111, 235691, firstLocSpec)]
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
                locSpec.py -E <planner path> -B <olg path> -T <time in seconds> 

                -E              planner path
                -B              olg path
		-T              time (secs) for test to run
                -h              prints usage message
        """


# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout_11111, stdout_22222):
	output = ""
        for line in stdout_11111.readlines():
		print line
		p = re.compile('^[#][#]Print.*$',re.DOTALL)
                if(p.match(line)):
                        output = output + line
	
	p = re.compile(r"""
		(^[#][#] Print \[AddAction!link!r5ECAMat!localhost:11111\]: \s*
		\[link \(localhost:22222, \s* localhost:11111, \s* 235691, \s* secondLocSpec \)\])
		""", re.VERBOSE)
	
	flag = p.match(output)
        if flag == 0:
		print "Test failed"
		return

	output = ""
        for line in stdout_22222.readlines():
                output = output + line

	p = re.compile(r"""
                (^[#][#] Print \[AddAction!link!r3_eca!localhost:22222 \]: \s*
		\[link \(localhost:22222, \s* localhost:11111, \s* 235691, \s* firstLocSpec \)\])
		""", re.VERBOSE)

        flag = p.match(output)
        if flag:
                print "Test passed"
                #print flag.group()
        else:
                print "Test failed"
                return

#Function to kill the child after a set time
def kill_pid(stdout_11111, stdout_22222, pid_11111, pid_22222):
        #print "killing child"
        os.kill(pid_11111, 3)
        os.kill(pid_22222, 3)
        #print "program killed"
        script_output(stdout_11111, stdout_22222)
	

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
        args=[executable_path , '-DLOCALID=\"localhost:11111\"', '-DNEIGHBORID=\"localhost:22222\"', '-DEVENTID=235691', '-o', os.path.join(olg_path, 'locSpec.olg'), '-n', 'localhost', '-p', '11111']
        p_11111 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
time.sleep(1)
pid_11111 = p_11111.pid
#print pid_11111

try:
	args=[executable_path , '-DLOCALID=\"localhost:22222\"', '-DNEIGHBORID=\"localhost:11111\"', '-DEVENTID=235691', '-o', os.path.join(olg_path, 'locSpec.olg'), '-n', 'localhost', '-p', '22222']
        p_22222 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
time.sleep(1)
pid_22222 = p_22222.pid
#print pid_22222


if os.getpid() != pid_11111 and os.getpid() != pid_22222:
        t = threading.Timer(int(time_interval), kill_pid, [p_11111.stdout, p_22222.stdout, pid_11111, pid_22222])
        t.start()
