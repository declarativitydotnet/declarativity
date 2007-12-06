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
# Given script runs stage.olg test and checks the test output
#
# Assumption - program is running at localhost:10000
#
# Expected output - (the order of the tuples should be same as listed below) ******** (DONT KNOW WHETHER abcd SHOULD BE FIRST or 1234)
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 1)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 2)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 3)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 4)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, a)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, b)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, c)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, d)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 1)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 2)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 3)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, 4)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, a)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, b)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, c)]
#	##Print[RecvEvent!outToken!outToken_watchStub!localhost:10000]:  [outToken(localhost:10000, d)]
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
                stage.py -E <planner path> -B <olg path>

                -E              planner path
                -B              olg path
                -h              prints usage message
        """


# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
        output = ""
        i = 1
	j = 1
	letters = ["a", "b", "c", "d"]
        result = 1
        for line in stdout.readlines():
                if j%2 == 1:
			p = re.compile(r"""
                        	(^[#][#]Print\[RecvEvent!outToken!outToken_watchStub!localhost:10000\]: \s* \[outToken\(localhost:10000, \s* """ + 
				str(i) + 
				"""\)\])
                        	""", re.VERBOSE|re.MULTILINE)
		else:
			#print letters[i-1]
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!outToken!outToken_watchStub!localhost:10000\]: \s* \[outToken\(localhost:10000, \s* """ +  
                                letters[i-1] + 
                                """\)\])
                                """, re.VERBOSE|re.MULTILINE)

                flag = p.match(line)
                if flag:
			if i == 4:
                        	j = j + 1
				i = 1
			else:
				i = i + 1
                else:
                        result = 0
                        break

	if result == 0 and j <= 4:
                print "Test failed"
        elif j > 5 or j < 5:
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
        args=[executable_path , '-o', olg_path +'/stage.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
