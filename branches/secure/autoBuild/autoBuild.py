#
# * This file is distributed under the terms in the attached LICENSE file.
# * If you do not find this file, copies can be found by writing to:
# * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# * Or
# * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776,
# * Berkeley, CA,  94707. Attention: P2 Group.
#

#!/usr/bin/python
import sys
import os
import datetime
import time 
import re 
import getopt
import string
import subprocess

# Usage function
def usage():                                            
	print """
		autobuild: Autobuild for nightly build of P2
		autobuild -C <configuration file directory path>

		-C		configuration file directory path.
 		-h              prints usage message
	"""
# Function to send mail with the log file information if the build failed
def failed (log, revision, fileHandle):
	fileHandle.close()
	mailfrom = parameters['from_address']
	mailheader = "To:" + parameters['to_address'] + "From: P2 Automated Build <" + mailfrom + ">"

	 # creating handle to log file
        fileHandle = open (sourcedir + "/" + log)
        text = fileHandle.read()
	fileHandle.close()

	SENDMAIL =  parameters['sendmail_path'] # sendmail location
	p = os.popen("%s -t -i" % SENDMAIL, "w")
	p.write("To: " + parameters['to_address'] + "\n")
	p.write("From: P2 Automated Build <" + mailfrom + ">\n")
	p.write("Subject: Build Failed : r" + revision + " - " + branch_name + "\n")
	p.write("Content-type: text/plain\n")
	p.write("\n") # blank line separating headers from body
	p.write(text + "\n")
	sts = p.close()
	if sts != 0:
    		print "Sendmail exit status", sts
	sys.exit(0)

# Function to send mail with the log file information
def succeeded(revision):
	mailfrom = parameters['from_address']
        mailheader = "To:" + parameters['to_address'] + "From: P2 Automated Build <" + mailfrom + ">"

	# creating handle to log file
	fileHandle = open (sourcedir + "/" + log)
	text = fileHandle.read()
        fileHandle.close()
	
	SENDMAIL = parameters['sendmail_path'] # sendmail location
        p = os.popen("%s -t -i" % SENDMAIL, "w")
        p.write("To: " + parameters['to_address'] + "\n")
        p.write("From: P2 Automated Build <" + mailfrom + ">\n")
        p.write("Subject: Build Succeeded : r" + revision  + " - " + branch_name + "\n")
	p.write("Content-type: text/plain\n")
        p.write("\n") # blank line separating headers from body
        p.write(text + "\n")
	sts = p.close()
        if sts != 0:
                print "Sendmail exit status", sts


#Get revsion number of the repository
def get_svn_revision(svnroot):
	revision = os.popen(svn_path + ' log \-rHEAD ' + svnroot)
	mod_rev = ""
	for file in revision.readlines():
        	file = file.replace("\n", "")
        	mod_rev = mod_rev + file

	revision_number =  (re.search("r[0-9]*\s",mod_rev)).group()
	revision.close()
	rev_final = revision_number[1:]
	return rev_final


#cmake function
def cmake(cmake_parameters, sourcedir, branch, fileHandle):
        cmd = parameters['cmake_path']
        for k in cmake_parameters.keys():
        	cmd = cmd + " " + k + " " + cmake_parameters[k] + " "
 
	print cmd
	#print os.getcwd()
        cmd = cmd + " "+ os.getcwd() + "/../"
        fileHandle.write("\n\nRunning Cmake \n")
	fileHandle.write("------------- \n")
	try:
		cmake_output = os.popen(cmd)
	except OSError, e:
                fileHandle.write( "Execution failed ") #print "Execution failed"
                fileHandle.write(str(e))
		sys.exit(0)
	except:
		fileHandle.write("error in cmake " + dir + "  "+ cmd); 
	
        for file in cmake_output.readlines():
                fileHandle.write(file)
	
	if cmake_output.close():
        	failed(log, revision, fileHandle)
	fileHandle.write("****************** SUCCESSFUL ******************")


#make function
def make(cmd, fileHandle):
	fileHandle.write("\n\nRunning make \n")
        fileHandle.write("------------ \n")
	make_output = os.popen(cmd)
	for file in make_output.readlines():
        	fileHandle.write(file)
	if make_output.close():
        	failed(log, revision, fileHandle)
	fileHandle.write("****************** SUCCESSFUL ******************")


#Function to run all test scripts
def run_scripts(opt, sourcedir, branch, fileHandle):
	#print os.getcwd()
	runOverLog_path = sourcedir + "/" + branch + "/builds/tests/" + planner
	olg_path = sourcedir + "/" + branch + "/unitTests/olg/"
	branch_path = sourcedir + "/" + branch
	script_path = sourcedir + "/" + branch + "/tests/unitTests/scripts/"
	#script_path = "/home/aatul/secure/tests/unitTests/scripts/"

	#print runOverLog_path
	#print olg_path
	#print branch_path
	
	fileHandle.write("\n\nExecuting scripts \n")
	fileHandle.write("----------------- \n\n")
	i = 1
	for f in os.listdir(olg_path):
    		if f.endswith('.olg'):
			try:
				fileHandle.write(str(i) + ". " + f + " - ")
				script_py = f.rstrip(".olg")
				script = script_path + script_py + ".py"
        			about = os.stat(script)
				
				args=[python_path, script, '-E', runOverLog_path, '-B', olg_path, '2>&1']
        			#print args
				p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
				for line in (p.stdout).readlines():
					fileHandle.write( line),
				i = i + 1
			except OSError, e:
                        	fileHandle.write( "Execution failed ") #print "Execution failed"
                                fileHandle.write(str(e))
				fileHandle.write("\n"),
				i = i + 1
        			#failed(log, revision, filehandle)	


def run_p2Test(sourcedir, branch, fileHandle):
	fileHandle.write("\n\nExecuting p2Test \n")
        fileHandle.write("----------------- \n\n")
        p2Test_path = sourcedir + "/" + branch + "/builds/unitTests/p2Test"
	print p2Test_path	
	args=[p2Test_path]
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
        for line in (p.stdout).readlines():
        	fileHandle.write( line),


def loadConfigParameters(file, parameters, cmake_parameters):
        fileHandle = open ( file, 'r' )
        temp = []
	flag = 0
        for line in fileHandle:
		if(line == "# CMAKE Parameters #\n"):
			flag = 1
		if (flag == 0):	
                	temp = line.split("=")
                	if len(temp) == 2:
                        	temp[1] = (temp[1].rstrip("\n")).lstrip()
                        	parameters[(temp[0].rstrip()).lstrip()] = temp[1]
		else:
			temp = line.split("=")
                        if len(temp) == 2:
                                temp[1] = (temp[1].rstrip("\n")).lstrip()
                                temp[1] = (temp[1].rstrip()).lstrip()
				if(temp[1] != ''):
					cmake_parameters[(temp[0].rstrip()).lstrip()] = temp[1]
	
	fileHandle.close()
        
	#print cmake_parameters
	#print parameters


def removeall(path):

    if not os.path.isdir(path):
        return
    
    files=os.listdir(path)

    for x in files:
        fullpath=os.path.join(path, x)
        if os.path.isfile(fullpath):
            f=os.remove
        elif os.path.isdir(fullpath):
            removeall(fullpath)
            f=os.rmdir

sourcedir = os.getcwd()

# Configuration variables
####### read configuration paramemters ########
parameters = {}
cmake_parameters = {}
opt, arg = getopt.getopt(sys.argv[1:], 'C:h')

for key,val in opt:
        if key == '-h':
                usage()
                sys.exit(0)
	if key == '-C':
		current_dir = os.getcwd()
		for f in os.listdir(val):
                	if f.endswith('.conf'):
				os.chdir(current_dir)
				path = os.path.join(val, f)	
				loadConfigParameters(path, parameters,cmake_parameters)
		
				svn_path = parameters['svn_path']       
				python_path = parameters['python_path'] 
				sendmail_path = parameters['sendmail_path'] 
				to_address = parameters['to_address']
				from_address = parameters['from_address']
				branch = parameters['branch']
				planner = parameters['planner']
				revision_number = parameters['revision']

				curr_time = datetime.datetime.now()
				EpochSeconds = time.mktime(curr_time.timetuple())
				date = datetime.datetime.fromtimestamp(EpochSeconds)
				
				if(revision_number.rstrip() == ""):
					revision = get_svn_revision(branch)
				else:
					revision = revision_number.rstrip()

				print "revision" , revision
				rev= date.strftime("%Y.%m.%d.%H.%M.%S")
				dir= "src_" + f.rstrip(".conf") + "_" + rev
				log= "LOG_" + f.rstrip(".conf") + "_" + rev


				# creating build directory
				if os.path.exists(dir) != True:
        				os.makedirs(dir)
				else:
					removeall(os.getcwd() + "/" + dir)	

				# changing cwd to build directory
				os.chdir(dir)

				sourcedir = os.getcwd()

				# creating handle to log file
				fileHandle = open (log, 'a' )
				fileHandle.write("Repository version:" +  revision)


				#if os.path.exists(branch) != True:
        			#	os.makedirs(branch)
				
				if(branch.rfind('/') == -1):
                                        print "Wrong branch path"
                                        sys.exit(0)
                                else:
                                        branch_name = branch[branch.rfind('/')+1:len(branch)]
                                        print branch_name
                                        os.mkdir(branch_name)

				#Changing current working directory to point to branch
				os.chdir(branch_name)

				#Update the branch from svn
				os.system(svn_path + " co -r " + revision + " " + branch + " ./")
	
				os.mkdir("builds")
				#changing working directory to local directory 
				os.chdir("builds")
				#print "Entered " + os.getcwd()

				cmake(cmake_parameters, sourcedir, branch_name, fileHandle)

				cmd = parameters['make_path'] + " -j4 2>&1"
				make(cmd, fileHandle)

				#running the scrips now
				run_scripts(opt, sourcedir, branch_name, fileHandle)
				run_p2Test(sourcedir, branch_name, fileHandle)

				fileHandle.close()

				succeeded(revision)
				if(parameters['keep_build'] == 'No'):
					os.chdir(current_dir)
					removeall(os.getcwd() + "/" + dir)
					os.rmdir(dir)
