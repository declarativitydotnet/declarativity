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
# Given script runs remoteView.olg test and checks the test output
#
# Assumptions -  Two nodes with ports 10001, 10000. Execute in the following order
#  
#	* At 10001:
#	* tests/runOverLog -o unitTests/olg/remoteView.olg -p 10001
#
#	* At 10000:
#	* tests/runOverLog -o unitTests/olg/remoteView.olg
#
#
# Expected output - 
#
#	* At 10001:
#	* ##Print[RecvEvent!out!out_watchStub!localhost:10001]:  [out(localhost:10001, 577655601)]
#
#	* At 10000: (No check for this right now)
#	* No output
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
                remoteView.py -E <planner path> -B <olg path>

                -E              planner path
                -B              olg path
                -h              prints usage message
        """


# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout_10001, stdout_10000):
        output = ""
        for line in stdout_10001.readlines():
                output = output + line
	
	p = re.compile(r"""
		(^[#][#] Print \[RecvEvent!out!out_watchStub!localhost:10001 \]: \s*
		\[out \(localhost:10001, \s* 577655601 \)\])
		""", re.VERBOSE)
	
	flag = p.match(output)
        if flag:
                print "Test passed"
                #print flag.group()
        else:
		print "Test failed"
		return
	
#Function to kill the child after a set time
def kill_pid(stdout_10001, stdout_10000 , pid_10001, pid_10000):
        #print "killing child"
        os.kill(pid_10001, 3)
        os.kill(pid_10000, 3)
        #print "program killed"
        script_output(stdout_10001, stdout_10000)


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
        args=[executable_path , '-o', olg_path +'/remoteView.olg', '-n', 'localhost', '-p', '10001', '2>&1']
        p_10001 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
time.sleep(1)
pid_10001 = p_10001.pid
#print pid_10001

try:
        args=[executable_path , '-o', olg_path +'/remoteView.olg', '-n', 'localhost', '-p', '10000', '2>&1']
        p_10000 = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)
time.sleep(1)
pid_10000 = p_10000.pid
#print pid_10000


if os.getpid() != pid_10000 and os.getpid() != pid_10001:
        t = threading.Timer(10, kill_pid, [p_10001.stdout, p_10000.stdout, pid_10001, pid_10000])
        t.start()
