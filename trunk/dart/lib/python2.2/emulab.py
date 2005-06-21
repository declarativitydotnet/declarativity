import os
import dart
import time
from dartpaths import *
from remotehost import RemoteHost
from tbreport import TBReport

class Emulab(RemoteHost):
    def __init__(self, project, experiment, nsfile, 
                 eipsfile, iipsfile, verbose=None):
        RemoteHost.__init__(self, "users.emulab.net", verbose)
        self.pid = project
        self.eid = experiment
        self.nsfile = nsfile
        self.eipsfile = eipsfile
        self.iipsfile = iipsfile
        self.verbose = verbose

    def mkexp(self):
        try:
            self.rmexp()
        except: pass
        basensfile = os.path.basename(self.nsfile)
        self.xfer_to(self.nsfile, basensfile)
	#self.rexec("batchexp -w -i -p %s -e %s %s" % \
        #           (self.pid, self.eid, basensfile))
        self.rexec("batchexp -w -p %s -e %s %s" % \
                   (self.pid, self.eid, basensfile))
        self.rexec("sshxmlrpc_client.py waitforactive proj=%s exp=%s" % \
                   (self.pid, self.eid))
        
    def rmexp(self):
        pass
# self.rexec("endexp -w %s %s" % (self.pid, self.eid))

    def setup(self):
        tb = TBReport(self.pid, self.eid, self.nsfile)
        self.write_ipfiles(tb)
        tmp = self.mkprofile(tb)
        h = RemoteHost(tb.eips[dart.MASTER_NODE])
        h.rexec("if [ ! -e %s ]; then mkdir %s; fi;" % \
                (DART_RESERVED_DIR, DART_RESERVED_DIR))
        h.xfer_to(tmp, DART_PROFILE)
        os.unlink(tmp)
        if self.verbose:
            for node in tb.nodes:
                print node, tb.iips[node], tb.eips[node]

    def read_ipfiles(self):
        eips = map(lambda(x): x.strip(), open(self.eipsfile).readlines())
        iips = map(lambda(x): x.strip(), open(self.iipsfile).readlines())
        return eips, iips

    def write_ipfiles(self, tb):
        f = open(self.eipsfile, "w")
        for node in tb.nodes:
            f.write("%s\n" % tb.eips[node])
        f.close()
        f = open(self.iipsfile, "w")
        for node in tb.nodes:
            f.write("%s\n" % tb.iips[node])
        f.close()

    def mkprofile(self, tb):
        """Note: GEXEC_SVRS (master + slaves), GEXEC_SLAVE_SVRS (slaves)"""
        import tempfile, StringIO
        profile = StringIO.StringIO()
        eips = map(lambda(node): tb.eips[node], tb.nodes)
        slaveeips = eips[1:]
        profile.write("export GEXEC_SVRS=\"%s\"\n" % " ".join(eips))
        profile.write("export GEXEC_SLAVE_SVRS=\"%s\"\n" % " ".join(slaveeips))
        tmp = tempfile.mktemp()
        open(tmp, "w").write(profile.getvalue())
        return tmp
