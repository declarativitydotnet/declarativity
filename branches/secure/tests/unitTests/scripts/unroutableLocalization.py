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
# Given script runs unroutableLocalization.olg test and checks the test output
#
# Assumption - program is runing at localhost:10000
#
# Expected output -
#	##Planner, -:-, 4, 0, Unable to localize rule 's1 separator(@I, J, V) :- clique(@I, V), clique(@J, V), I != J.'. I don't have enough information to forward partial results to subsequent nodes. If the right hand side of the rule has something like 'x(@A, B), y(@C, D)', make sure that 'C' is one of 'A' or 'B'.
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
                unroutableLocalization.py -E <planner path> -B <olg path>

                -E              planner path
                -B              olg path
                -h              prints usage message
        """


# Function to parse the output file and check whether the output matches the expected value
def script_output(stdout):
        output = ""
        for line in stdout.readlines():
                output = output + line
	
	p = re.compile(r"""
		(^[#][#]Planner, \s* [-]:[-], \s* 4, \s* 0, \s* Unable \s* to \s* localize \s* rule \s* ['] s1 \s* separator\(@I, \s* J, \s* V\)\s* :[-] \s* clique\(@I, \s* V\), \s* clique\(@J, \s* V\), \s* I \s* [!][=] \s* J[.]['][.] \s* I \s* don[']t \s* have \s* enough \s* information \s* to \s* forward \s* partial \s* results \s* to \s* subsequent \s* nodes[.] \s* If \s* the \s* right \s* hand \s* side \s* of \s* the \s* rule \s* has \s* something \s* like \s* [']x\(@A, \s* B\), \s* y\(@C, \s* D\)['], \s* make \s* sure \s* that \s* [']C['] \s* is \s* one \s* of \s* [']A['] \s* or \s* [']B['][.])
		""", re.VERBOSE)

	flag = p.match(output)
	if flag:
		print "Test passed"
		#print flag.group()
	else:
		print "Test failed"


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
        args=[executable_path , '-o', olg_path +'/unroutableLocalization.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
