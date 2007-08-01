import os
import re
import resolve
import sys
import shutil
import socket
import tempfile
import StringIO
import dart
import dartenv
from dartpaths import *
from remotehost import RemoteHost

PCP_GATHER_GROUPSIZE        = 8 # Max number of parallel gather nodes

DIST_ENV_MASTER_SCRIPT      = "%s/master_distenv.sh" % DART_RESERVED_DIR
DIST_APP_MASTER_SCRIPT      = "%s/master_distapp.sh" % DART_RESERVED_DIR
DIST_APP_SLAVES_SCRIPT      = "%s/slaves_distapp.sh" % DART_RESERVED_DIR
DIST_TEST_MASTER_SCRIPT     = "%s/master_disttest.sh" % DART_RESERVED_DIR
PREEXECUTION_ALL_SCRIPT     = "%s/all_preexecution.sh" % DART_RESERVED_DIR
EXECUTION_MASTER_SCRIPT     = "%s/master_execution.sh" % DART_RESERVED_DIR
COLL_OUT_ALL_SCRIPT         = "%s/all_collout.sh" % DART_RESERVED_DIR
COLL_OUT_MASTER_SCRIPT      = "%s/master_collout.sh" % DART_RESERVED_DIR
POSTEXECUTION_MASTER_SCRIPT = "%s/all_postexecution.sh" % DART_RESERVED_DIR
RESET_ALL_SCRIPT            = "%s/all_reset.sh" % DART_RESERVED_DIR

class Master(RemoteHost):
    def __init__(self, dart, verbose=None):
        RemoteHost.__init__(self, dart.mastereip, verbose=verbose)
        self.dart = dart
        self.verbose = verbose

    def do_all(self, cmd, cmddata=None, detached=""):
        """Remotely execute cmd on master/slaves through master"""
        if cmddata:
            tmp = tempfile.mktemp()
            s = StringIO.StringIO()
            s.write("source %s\n" % DART_ENV)
            if not os.path.isabs(cmd):
                raise "Must specify absolute path w/ cmddata for GEXEC"
            s.write("%s\n" % cmddata)
            open(tmp, "w").write(s.getvalue())
            self.xfer_to(tmp, cmd)
            self.rexec("(source %s; pcp %s %s \$GEXEC_SLAVE_SVRS; gexec %s -n 0 sh %s;)" % (DART_PROFILE, cmd, cmd, detached, cmd))
            os.unlink(tmp)
        else: # Note: master/slaves don't have DART environment in this case
            self.rexec("(source %s; gexec %s -n 0 %s;)" % (DART_PROFILE, detached, cmd))

    def do_some(self, nodes, cmd, cmddata=None, detached=""):
        nodesstr = " ".join(nodes)
        slavenodes = []
        for n in nodes:
            if n != self.dart.mastereip:
                slavenodes.append(n)
        slavenodesstr = " ".join(slavenodes)
        n = len(nodes)
        if cmddata:
            tmp = tempfile.mktemp()
            s = StringIO.StringIO()
            s.write("source %s\n" % DART_ENV)
            if not os.path.isabs(cmd):
                raise "Must specify absolute path w/ cmddata for GEXEC"
            s.write("%s\n" % cmddata)
            open(tmp, "w").write(s.getvalue())
            self.xfer_to(tmp, cmd)
            self.rexec("(source %s; pcp %s %s %s; env GEXEC_SVRS=\\\"%s\\\" gexec %s -n %d sh %s;)" % (DART_PROFILE, cmd, cmd, slavenodesstr, nodesstr, detached, n, cmd))
            os.unlink(tmp)
        else: # Note: master/slaves don't have DART environment in this case
            self.rexec("(source %s; env GEXEC_SVRS=\\\"%s\\\" gexec %s -n %d %s;)" % (DART_PROFILE, nodesstr, detached, n, cmd))

    def do_master(self, cmd, cmddata=None):
        """Remotely execute cmd on master"""
        if cmddata:
            if not os.path.isabs(cmd):
                raise "Must specify absolute path w/ cmddata for GEXEC"
            tmp = tempfile.mktemp()
            s = StringIO.StringIO()
            s.write("%s\n" % cmddata)
            open(tmp, "w").write(s.getvalue())
            self.xfer_to(tmp, cmd)
            os.unlink(tmp)
            self.rexec("(source %s; sh %s;)" % (DART_PROFILE, cmd))
        else:
            self.rexec("(source %s; %s;)" % (DART_PROFILE, cmd))

    def do_slaves(self, cmd, cmddata=None, detached=""):
        """Remotely execute cmd on slaves through master"""
        if cmddata:
            tmp = tempfile.mktemp()
            s = StringIO.StringIO()
            s.write("source %s\n" % DART_ENV)
            if not os.path.isabs(cmd):
                raise "Must specify absolute path w/ cmddata for GEXEC"
            s.write("%s\n" % cmddata)
            open(tmp, "w").write(s.getvalue())
            self.xfer_to(tmp, cmd)
            self.rexec("(source %s; pcp %s %s \$GEXEC_SLAVE_SVRS; env GEXEC_SVRS=\\\"\$GEXEC_SLAVE_SVRS\\\" gexec %s -n 0 sh %s;)" % (DART_PROFILE, cmd, cmd, detached, cmd))
            os.unlink(tmp)
        else: # Note: slaves don't have DART environment in this case
            self.rexec("(source %s; env GEXEC_SVRS=\\\"\$GEXEC_SLAVE_SVRS\\\" gexec %s -n 0 %s;)" % (DART_PROFILE, detached, cmd))

    def read_master(self, path):
        """Read a remote file from master"""
        tmp = tempfile.mktemp()
        self.xfer_from(path, tmp)
        data = open(tmp).read()
        os.unlink(tmp)
        return data

    def dist_env_files(self):
        """Distribute DART environment variables files to all nodes"""
        self.do_slaves("mkdir %s" % DART_RESERVED_DIR)
        tmpdir = tempfile.mktemp()
        os.mkdir(tmpdir)
        for vnn in range(self.dart.numnodes):
            path = "%s/%s" % (tmpdir, self.dart.eips[vnn])
            e = dartenv.DartEnv(self.dart.df.name, self.dart.iips,
                                self.dart.masteriip, self.dart.mastereip, vnn)
            open(path, "w").write(e.sub(dartenv.DART_ENV_TEMPLATE))
        self.xfer_to(tmpdir, dart.DART_ENVDIR)
        s = StringIO.StringIO()
        for eip in os.listdir(tmpdir):
            s.write("pcp %s/%s %s %s\n" % \
                    (dart.DART_ENVDIR, eip, dart.DART_ENV, eip))
        self.do_master(DIST_ENV_MASTER_SCRIPT, cmddata=s.getvalue())
        shutil.rmtree(tmpdir)

    def dist_app_files(self):
        """Distribute application files to all nodes"""
        commondirs = self.dart.df.commondirs
        commonfiles = self.dart.df.commonfiles
        # Transfer application files to master
        for src in commondirs:
            cd = commondirs[src]
            if cd["type"] == "local":
                self.xfer_to(src, cd["dst"])
            else:
                self.rexec("cp -r %s %s" % (self.host, src, cd["dst"]))
        for src in commonfiles:
            cf = commonfiles[src]
            if cf["type"] == "local":
                self.xfer_to(src, cf["dst"])
            else:
                self.rexec("cp %s %s" % (src, cf["dst"]))
        # Transfer application tarfiles from master to slaves
        s = StringIO.StringIO()
        s.write("gexec -n 0 mkdir %s\n" % DART_MY_INPUT_DIR)
        s.write("gexec -n 0 mkdir %s\n" % DART_MY_OUTPUT_DIR)
        s.write("gexec -n 0 mkdir %s\n" % DART_ALL_OUTPUT_DIR)
        for cd in commondirs.values():
            dstdir, dstname = os.path.split(cd["dst"])
            tarfile = "/tmp/%s.tar.gz" % dstname
            s.write("cd %s\n" % dstdir)
            s.write("tar czf %s %s\n" % (tarfile, dstname))
            s.write("pcp %s %s $GEXEC_SLAVE_SVRS\n" % (tarfile, tarfile))
        for cf in commonfiles.values():
            s.write("pcp %s %s $GEXEC_SLAVE_SVRS\n" % (cf["dst"], cf["dst"]))
        self.do_master(DIST_APP_MASTER_SCRIPT, cmddata=s.getvalue())
        # Untar application tarfiles (if any) on slaves
        if len(commondirs) > 0:
            s = StringIO.StringIO()
            for cd in commondirs.values():
                dstdir, dstname = os.path.split(cd["dst"])
                tarfile = "/tmp/%s.tar.gz" % dstname
                s.write("cd %s\n" % dstdir)
                s.write("tar xfz %s\n" % tarfile)
            self.do_slaves(DIST_APP_SLAVES_SCRIPT, cmddata=s.getvalue())

    def dist_test_files(self):
        """Distribute test files to all nodes"""
        # Transfer test files to master
        tmpdir = tempfile.mktemp()
        os.mkdir(tmpdir)
        for i in range(len(self.dart.df.execution)):
            s = StringIO.StringIO()
            s.write("source %s\n" % DART_ENV)
            s.write("%s\n" % self.dart.df.execution[i]["cmd"])
            open("%s/%04d" % (tmpdir, i), "w").write(s.getvalue())
        self.xfer_to(tmpdir, DART_TEST_SCRIPTS_DIR)
        shutil.rmtree(tmpdir)
        # Transfer test tarfiles from master to slave and untar on slaves
        dirname, basename = os.path.split(DART_TEST_SCRIPTS_DIR)
        tarfile = "%s.tar.gz" % basename
        s = StringIO.StringIO()
        s.write("cd %s\n" % dirname)
        s.write("tar cfz %s %s\n" % (tarfile, basename))
        s.write("pcp %s/%s %s/%s $GEXEC_SLAVE_SVRS\n" % (dirname, tarfile, dirname, tarfile))
        s.write("env GEXEC_SVRS=\"$GEXEC_SLAVE_SVRS\" gexec -n 0 tar -C %s -zxf %s" % (dirname, tarfile))
        self.do_master(DIST_TEST_MASTER_SCRIPT, cmddata=s.getvalue())
    
    def preexecution(self):
        """Run preexecution script (if any) on all nodes"""
        if self.dart.df.preexecution:
            s = StringIO.StringIO()
            s.write("sh %s\n" % self.dart.df.preexecution)
            self.do_all(PREEXECUTION_ALL_SCRIPT, s.getvalue())

    def execute(self):
        """Run test files (at specific times) on all nodes"""
        s = StringIO.StringIO()
        # Note: using GEXEC detached mode for all test files
        for i in range(len(self.dart.df.execution)):
            e = self.dart.df.execution[i]
            eips = self.dart.map_vnns(e["vnns"], dart.EXTERNAL_IPS)
            numnodes = len(eips)
            s.write("(sleep %d; " % e["time"])
            s.write(" export GEXEC_SVRS=\"%s\";" % (" ".join(eips)))
            s.write(" gexec -d -n %d sh %s/%04d) &\n" % \
                    (numnodes, DART_TEST_SCRIPTS_DIR, i))
        s.write("sleep %d\n" % self.dart.df.duration)
#        self.dart.emulab_faults() # NOTE: no setsid on Emulab!
        self.do_master(EXECUTION_MASTER_SCRIPT, cmddata=s.getvalue())
        
    def collect_out_files(self):
        """Collect output files from all nodes"""
        # Create output tarfiles on all nodes
        dirname, basename = os.path.split(DART_MY_OUTPUT_DIR)
        self.do_all("tar -C %s -zcf %s/%s.tar.gz ." % \
                    (DART_MY_OUTPUT_DIR, dirname, basename))

        # Transfer output tarfiles to master (bound parallelism on gather)
        nodes = self.dart.eips[:]
        while nodes:
            s = StringIO.StringIO()
            s.write("pcp %s/%s.tar.gz %s/$DART_MY_VNN.tar.gz $DART_GEXEC_MASTER\n" % (dirname, basename, DART_ALL_OUTPUT_DIR))
            if len(nodes) >= PCP_GATHER_GROUPSIZE:
                pcpgroup = nodes[0:PCP_GATHER_GROUPSIZE]
                nodes = nodes[PCP_GATHER_GROUPSIZE:]
            else:
                pcpgroup = nodes[:]
                nodes = []
            self.do_some(pcpgroup, COLL_OUT_ALL_SCRIPT, s.getvalue())
        
        # Untar output tarfiles on master
        s = StringIO.StringIO()
        s.write("cd %s\n" % DART_ALL_OUTPUT_DIR)
        for vnn in range(self.dart.numnodes):
            s.write("rm -rf %d\n" % vnn)
            s.write("mkdir %d\n" % vnn)
            s.write("tar -C %d -zxf %d.tar.gz\n" % (vnn, vnn))
            s.write("rm -f %d.tar.gz\n" % vnn)
        self.do_master(COLL_OUT_MASTER_SCRIPT, cmddata=s.getvalue())
        
        # Optionally copy output files to desktop
        if self.dart.df.outdir:
            self.xfer_from("%s/" % DART_ALL_OUTPUT_DIR, self.dart.df.outdir)

    def postexecution(self):
        """Run postexecution script on master"""
        s = StringIO.StringIO()
        s.write("source %s\n" % DART_ENV)
        s.write("%s >& %s\n" % (self.dart.df.postexecution, DART_TEST_ERROR_MSG))
        s.write("echo $? > %s\n" % DART_TEST_RESULT)
        self.do_master(POSTEXECUTION_MASTER_SCRIPT, s.getvalue())
        rval = int(self.read_master(DART_TEST_RESULT))
        if not rval:
            print "Test %s OK" % (self.dart.df.name)
        else:
            print "Test %s ERROR %d" % (self.dart.df.name, rval)
            errmsg = self.read_master(DART_TEST_ERROR_MSG)
            for line in errmsg.split("\n"):
                print "   %s" % line
            
    def setup(self):
        self.dist_env_files()
        self.dist_app_files()
        self.dist_test_files()
       
    def run(self):
        self.preexecution()
        self.execute()
        self.collect_out_files()    
        self.postexecution()

    def reset(self):
        s = StringIO.StringIO()
        s.write("sudo sh %s\n" % self.dart.df.reset)
        s.write("sudo killall sh\n") # DART cmds (e.g., EXECUTION_MASTER_SCRIPT)
        s.write("sudo killall sleep\n")
        self.do_all(RESET_ALL_SCRIPT, s.getvalue())
