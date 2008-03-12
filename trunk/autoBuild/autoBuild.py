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
		autobuild -C <configuration file directory path> -F <configuration file path>
		
		-C		configuration file directory path.
		-F		configuration file path
 		-h              prints usage message
	"""
# Function to send mail with the log file information if the build failed
def failed (revision, branch_name):
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
def succeeded(revision, branch_name):
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

def email(from_address, to_address, flag, log, sendmail_path, revision, branch_name, log_path):
        mailheader = "To:" + to_address + "From: P2 Automated Build <" + from_address + ">"

        # creating handle to log file
        fileHandle = open (os.path.join(log_path, log))
        text = fileHandle.read()
        fileHandle.close()

        SENDMAIL = sendmail_path # sendmail location
        p = os.popen("%s -t -i" % SENDMAIL, "w")
        p.write("To: " + to_address + "\n")
        p.write("From: P2 Automated Build <" + from_address + ">\n")
	if flag:
        	p.write("Subject: Build Succeeded : r" + revision  + " - " + branch_name + "\n")
        else:
		p.write("Subject: Build Failed : r" + revision  + " - " + branch_name + "\n")
	p.write("Content-type: text/plain\n")
        p.write("\n") # blank line separating headers from body
        p.write(text + "\n")
        sts = p.close()
        if sts != 0:
                print "Sendmail exit status", sts
 

#Get revsion number of the repository
def get_svn_revision(svnroot, svn_path):
	revision = os.popen(svn_path + ' info -rHEAD ' + svnroot + ' |grep "^Revision"')
        mod_rev = ""
        for file in revision.readlines():
                print file
                #file = file.replace("\n", "")
                mod_rev = mod_rev + file
        revision_number =  (re.search("Revision:\s*[0-9]*\s",mod_rev)).group()
        revision.close()
        revision_number = revision_number.lstrip().rstrip()
        rev_final = revision_number[revision_number.rfind(" ")+1:]
        return rev_final

#cmake function
def cmake(cmake_parameters, sourcedir, branch, fileHandle, cmake_path, verbose_output):
        cmd = cmake_path
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
	
	if (verbose_output == "Yes") or cmake_output.close():
        	for file in cmake_output.readlines():
                	fileHandle.write(file)
	
	if cmake_output.close():
		return 0
                #failed(revision, branch_name)
	fileHandle.write("****************** SUCCESSFUL ******************")
        return 1

#make function
def make(cmd, fileHandle, verbose_output):
	fileHandle.write("\n\nRunning make \n")
        fileHandle.write("------------ \n")
	make_output = os.popen(cmd)
	for file in make_output.readlines():
        	fileHandle.write(file)
	if (verbose_output == "Yes") or make_output.close():
                for file in make_output.readlines():
                        fileHandle.write(file)

	if make_output.close():
        	return 0
		#failed(revision, branch_name)
	fileHandle.write("****************** SUCCESSFUL ******************")
	return 1

#Function to run all test scripts
def run_scripts(runOverLog_path, olg_path, script_path, fileHandle, python_path, verbose_output, wait_delay):

	#print runOverLog_path
	#print olg_path
	#print script_path
	
	fileHandle.write("\n\nExecuting scripts \n")
	fileHandle.write("----------------- \n\n")
	i = 1
	fail = 0
	file_list = os.listdir(olg_path)
	file_list.sort()
	for f in file_list:
    		if f.endswith('.olg'):
			try:
				script_py = f.rstrip(".olg")
				script = os.path.join(script_path, script_py) + ".py"
        			print "Running", script
				about = os.stat(script)
				about = os.stat(runOverLog_path)
			
				args=[python_path, script, '-E', runOverLog_path, '-B', olg_path, '-T', wait_delay, '2>&1']
        			#print args
				p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
		
				regExp = re.compile(r"""
					(^\s*test \s* passed\s*)
					""", re.VERBOSE|re.IGNORECASE)

				output = ""
				for line in (p.stdout).readlines():
					output = output + line
				print f, output
				flag = regExp.match(output)
				if(not flag):	
					fileHandle.write(str(i) + ". " + f + " - ")
					fileHandle.write( output)
					i = i + 1
					fail = 1
				elif(verbose_output == 'Yes'):
					fileHandle.write(str(i) + ". " + f + " - ")
                                        fileHandle.write( output)
                                        i = i + 1

			except OSError, e:
				fileHandle.write(str(i) + ". " + f + " - ")
                        	fileHandle.write( "Execution failed ") #print "Execution failed"
                                fileHandle.write(str(e))
				fileHandle.write("\n"),
				i = i + 1
				fail = 1
        			#failed(log, revision, filehandle)	
	
	if fail:
		return 0
	else:
		return 1

def run_p2Test(build_path, fileHandle):
	fileHandle.write("\n\nExecuting p2Test \n")
        fileHandle.write("----------------- \n\n")
	print "Executing p2Test\n"
        p2Test_path =os.path.join(build_path, 'tests/unitTests/p2Test')
	#print p2Test_path	
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

def autoBuild(file_path, file, current_dir):
        parameters = {}
	cmake_parameters = {}
        loadConfigParameters(file_path, parameters,cmake_parameters)

        svn_path = parameters['svn_path']
        python_path = parameters['python_path']
        sendmail_path = parameters['sendmail_path']
        to_address = parameters['to_address']
        from_address = parameters['from_address']
        branch = parameters['branch']
        planner = parameters['planner']
        revision_number = parameters['revision']
        build_path = parameters['build_path'].rstrip().lstrip()
        unitTests_path = parameters['unitTests_path'].rstrip().lstrip()
        run_build = parameters['run_build'].rstrip()
        verbose_output = parameters['verbose_result'].rstrip().lstrip()
	wait_delay = parameters['delay']

	print "Delay set to" , wait_delay

	curr_time = datetime.datetime.now()
        EpochSeconds = time.mktime(curr_time.timetuple())
        date = datetime.datetime.fromtimestamp(EpochSeconds)

	if(run_build == 'No' and (build_path == "" or unitTests_path == "")):
                print "Build path and unitTests path have to be specified if run_build option is set to No"
                sys.exit(0)

	if(run_build == 'Yes'):
        	if(revision_number.rstrip().lstrip() == ""):
        		revision = get_svn_revision(branch, svn_path)
        	else:
                	revision = revision_number.rstrip()

        	print "revision" , revision
                rev= date.strftime("%Y.%m.%d.%H.%M.%S")
                dir= "src_" + file.rstrip(".conf") + "_" + rev
                log= "LOG_" + file.rstrip(".conf") + "_" + rev

        	# creating build directory
        	if os.path.exists(dir) != True:
                	os.makedirs(dir)
                else:
                        removeall(os.apth.join(os.getcwd(), dir))

		# changing cwd to build directory
                os.chdir(dir)

                sourcedir = os.getcwd()

                # creating handle to log file
                fileHandle = open (log, 'a' )
                fileHandle.write("Repository version:" +  revision)

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
                
		result = cmake(cmake_parameters, sourcedir, branch_name, fileHandle, parameters['cmake_path'], verbose_output)
		if(result == 0):
			email(from_address, to_address, 0, log, sendmail_path, revision, branch_name, sourcedir)
			sys.exit(0)

                cmd = parameters['make_path'] + " -j4 2>&1"
                result = make(cmd, fileHandle, verbose_output)
		if(result == 0):
                        email(from_address, to_address, 0, log, sendmail_path, revision, branch_name, sourcedir)
                        sys.exit(0)
		
                runOverLog_path = os.path.join(sourcedir, branch_name, 'build/bin', planner)
                olg_path = os.path.join(sourcedir, branch_name, 'tests/unitTests/olg')
                script_path = os.path.join(sourcedir, branch_name, 'tests/unitTests/scripts')

                result = run_scripts(runOverLog_path, olg_path, script_path, fileHandle, python_path, verbose_output, wait_delay)
	        
		build_path = os.path.join(sourcedir, branch_name, 'builds')
                run_p2Test(build_path, fileHandle)
                
		fileHandle.close()
              
		if(parameters['to_address'] != '' and parameters['from_address'] != ''):
			if(result == 0):
                        	email(from_address, to_address, 0, log, sendmail_path, revision, branch_name, sourcedir)
			else:
                  		email(from_address, to_address, 1, log, sendmail_path, revision, branch_name, sourcedir)
                
		if(parameters['keep_build'] == 'No'):
                      	os.chdir(current_dir)
                        removeall(os.path.join(os.getcwd(), dir))
                        os.rmdir(dir)
		
	else:
        	# creating handle to log file
                result = 1
		rev= date.strftime("%Y.%m.%d.%H.%M.%S")
                log= "LOG_" + file.rstrip(".conf") + "_" + rev
                fileHandle = open (log, 'a' )
                fileHandle.write("Build:" +  build_path)

                runOverLog_path = os.path.join(build_path, 'bin', planner)
                olg_path = os.path.join(unitTests_path, 'olg')
                script_path = os.path.join(unitTests_path, 'scripts')
                result = run_scripts(runOverLog_path, olg_path, script_path, fileHandle, python_path, verbose_output, wait_delay)

                run_p2Test(build_path, fileHandle)
		
                fileHandle.close()
                if(parameters['to_address'] != '' and parameters['from_address'] != ''):
                        if(result == 0):
				email(from_address, to_address, 0, log, sendmail_path, "", build_path, os.getcwd())
			else:
				email(from_address, to_address, 1, log, sendmail_path, "", build_path, os.getcwd())
                print "tests have been run"


# Remove dir code from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/193736
ERROR_STR= """Error removing %(path)s, %(error)s """

def rmgeneric(path, __func__):

    try:
        __func__(path)
        #print 'Removed ', path
    except OSError, (errno, strerror):
        print ERROR_STR % {'path' : path, 'error': strerror }
            
def removeall(path):

    if not os.path.isdir(path):
        return
    
    files=os.listdir(path)

    for x in files:
        fullpath=os.path.join(path, x)
        if os.path.isfile(fullpath):
            f=os.remove
            rmgeneric(fullpath, f)
        elif os.path.isdir(fullpath):
            removeall(fullpath)
            f=os.rmdir
            rmgeneric(fullpath, f)

sourcedir = os.getcwd()

# Configuration variables
####### read configuration paramemters ########
pathname = os.path.dirname(sys.argv[0]) 
print pathname
#os.chdir(os.path.join(os.getcwd(), pathname))

current_dir = os.getcwd()
#opt, arg = getopt.getopt(sys.argv[1:], 'C:F:h')
try:
        opt, arg = getopt.getopt(sys.argv[1:], 'C:F:h')
except getopt.GetoptError:
        usage()
        sys.exit(2)

if(len(opt) == 0):
	usage()
	sys.exit(1)

for key,val in opt:
        if key == '-h':
                usage()
                sys.exit(0)
	elif key == '-C':
		for f in os.listdir(os.path.join(os.getcwd(), val)):
                	if f.endswith('.conf'):
				os.chdir(current_dir)
				path = os.path.join(val, f)	
				autoBuild(path, f, current_dir)
	elif key == '-F':
		 os.chdir(current_dir)	
		 path = os.path.join(os.getcwd(), val)	
		 if (val.rfind('/') == -1):
			f = val
                 else:
                        f = val[val.rfind('/')+1:len(val)]
		 autoBuild(path, f, current_dir)
	else:
		usage()
                sys.exit(0)	
