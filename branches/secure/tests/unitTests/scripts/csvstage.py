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
# Given script runs csvstage.olg test and checks the test output
#
# Assumption - program is running at localhost:10000
#
#Expected output - (the order of the tuples can vary)
#	##Print[SendAction!out!0-localhost:10000!localhost:10000]:  [out(localhost:10000, 0, boon, 14000)]
#	##Print[RecvEvent!out!out_watchStub!localhost:10000]:  [out(localhost:10000, 0, boon, 14000)]
#	##Print[SendAction!out!0-localhost:10000!localhost:10000]:  [out(localhost:10000, 1, tyson, 1300)]
#	##Print[HeadProjection!out!out_watchStub!localhost:10000]:  [out(localhost:10000)]
#	##Print[RecvEvent!out!out_watchStub!localhost:10000]:  [out(localhost:10000, 1, tyson, 1300)]
#	##Print[SendAction!out!0-localhost:10000!localhost:10000]:  [out(localhost:10000, 2, alex, 400)]
#	##Print[HeadProjection!out!out_watchStub!localhost:10000]:  [out(localhost:10000)]
#	##Print[RecvEvent!out!out_watchStub!localhost:10000]:  [out(localhost:10000, 2, alex, 400)]
#	##Print[SendAction!out!0-localhost:10000!localhost:10000]:  [out(localhost:10000, 3, atul, 200)]
#	##Print[HeadProjection!out!out_watchStub!localhost:10000]:  [out(localhost:10000)]
#	##Print[RecvEvent!out!out_watchStub!localhost:10000]:  [out(localhost:10000, 3, atul, 200)]
#	##Print[SendAction!out!0-localhost:10000!localhost:10000]:  [out(localhost:10000, 4, eric, 100)]
#	##Print[HeadProjection!out!out_watchStub!localhost:10000]:  [out(localhost:10000)]
#	##Print[RecvEvent!out!out_watchStub!localhost:10000]:  [out(localhost:10000, 4, eric, 100)]
#	##Print[SendAction!out!0-localhost:10000!localhost:10000]:  [out(localhost:10000, 5, alex, 50)]
#	##Print[HeadProjection!out!out_watchStub!localhost:10000]:  [out(localhost:10000)]
#	##Print[RecvEvent!out!out_watchStub!localhost:10000]:  [out(localhost:10000, 5, alex, 50)]
#	##Print[HeadProjection!out!out_watchStub!localhost:10000]:  [out(localhost:10000)]
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
                csvstage.py -E <planner path> -B <olg path>

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
				(^[#][#]Print\[HeadProjection!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000\)\])
                        	""", re.VERBOSE)
		elif i == 2:
			p = re.compile(r"""
				(^[#][#]Print\[HeadProjection!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 3:
			p = re.compile(r"""
                                (^[#][#]Print\[HeadProjection!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 4:
			p = re.compile(r"""
                                (^[#][#]Print\[HeadProjection!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 5:
			p = re.compile(r"""
                                (^[#][#]Print\[HeadProjection!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000\)\])
                                """, re.VERBOSE)
		elif i == 6:
			p = re.compile(r"""
                                (^[#][#]Print\[HeadProjection!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000\)\])
				""", re.VERBOSE)
		elif i == 7:
			p = re.compile(r"""
				(^[#][#]Print\[RecvEvent!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000, \s* 0, \s* boon, \s* 14000\)\])
				""", re.VERBOSE)
		elif i == 8:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000, \s* 1, \s* tyson, \s* 1300\)\])
                                """, re.VERBOSE)
		elif i == 9:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000, \s* 2, \s* alex, \s* 400\)\])
                                """, re.VERBOSE)
		elif i == 10:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000, \s* 3, \s* atul, \s* 200\)\])
                                """, re.VERBOSE)
		elif i == 11:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000, \s* 4, \s* eric, \s* 100\)\])
                                """, re.VERBOSE)
		elif i == 12:
			p = re.compile(r"""
                                (^[#][#]Print\[RecvEvent!out!out_watchStub!localhost:10000\]: \s* \[out\(localhost:10000, \s* 5, \s* alex, \s* 50\)\])
                                """, re.VERBOSE)
		elif i == 13:
			p = re.compile(r"""
				(^[#][#]Print\[SendAction!out!0[-]localhost:10000!localhost:10000\]: \s* \[out\(localhost:10000, \s* 0, \s* boon, \s* 14000\)\])
                                """, re.VERBOSE)
		elif i == 14:
			p = re.compile(r"""
                                (^[#][#]Print\[SendAction!out!0[-]localhost:10000!localhost:10000\]: \s* \[out\(localhost:10000, \s* 1, \s* tyson, \s* 1300\)\])
                                """, re.VERBOSE)
		elif i == 15:
                        p = re.compile(r"""
                                (^[#][#]Print\[SendAction!out!0[-]localhost:10000!localhost:10000\]: \s* \[out\(localhost:10000, \s* 2, \s* alex, \s* 400\)\])
                                """, re.VERBOSE)
		elif i == 16:
                        p = re.compile(r"""
                                (^[#][#]Print\[SendAction!out!0[-]localhost:10000!localhost:10000\]: \s* \[out\(localhost:10000, \s* 3, \s* atul, \s* 200\)\])
                                """, re.VERBOSE)
		elif i == 17:
                        p = re.compile(r"""
                                (^[#][#]Print\[SendAction!out!0[-]localhost:10000!localhost:10000\]: \s* \[out\(localhost:10000, \s* 4, \s* eric, \s* 100\)\])
                                """, re.VERBOSE)
		elif i == 18:
                        p = re.compile(r"""
                                (^[#][#]Print\[SendAction!out!0[-]localhost:10000!localhost:10000\]: \s* \[out\(localhost:10000, \s* 5, \s* alex, \s* 50\)\])
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
	
	if i>19 or i<19:
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
        args=[executable_path , '-o', olg_path +'/csvstage.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
