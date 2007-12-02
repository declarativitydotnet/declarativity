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
# Given script runs multiResultAggregate.olg test and checks the test output
#
# Assumption - program is running at localhost:10000
#
# Expected output -
#	##Planner, -:-, 4, 0, PLANNER ERROR: Group-by field E does not come from the event in the aggregation rule.  Currently, only group-by fields from the event are supported for event-table aggregates,  for rule ECA Rule r1_eca ACTION_SEND<matches(@A, E, COUNT< * >)> :- EVENT_RECV<event(@A, B, C)>, table(@A, C, D, E).. Planner exits.
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
                multiResultAggregate.py -E <planner path> -B <olg path>

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
                (^[#][#]Planner, .*
                PLANNER \s* ERROR: .*
                ECA \s* Rule \s* r1_eca \s* ACTION_SEND \<matches\(@A, \s* E, \s* COUNT \< \s* \* \s* \>\)\> \s* :- \s* 
		EVENT_RECV\<event\(@A, \s* B, \s* C\)\>, \s* table\(@A, \s* C, \s* D, \s* E\) .*)
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
        args=[executable_path , '-o', olg_path +'/multiResultAggregate.olg', '2>&1']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
except OSError, e:
        #print "Execution failed"
        print e
        sys.exit(0)

#print p.pid

if os.getpid() != p.pid:
        t = threading.Timer(3, kill_pid, [p.stdout, p.pid])
        t.start()
