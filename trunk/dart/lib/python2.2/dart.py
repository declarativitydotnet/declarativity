import os
import re
import socket
import sys
import time
import xml.dom.minidom
import StringIO
from dartpaths import *
from emulab import Emulab
from master import Master
from util import gettext

EXTERNAL_IPS     = 0 # Control network
INTERNAL_IPS     = 1 # Emulated network
MASTER_NODE      = "node0000" 
XML_SRC_TYPES    = [ "local", "remote" ]
XML_SRC_DEFAULT  = "local"

class DartFile:
    def __init__(self, dartfile):
        self.dartfile = dartfile
        self.preprocessed = ""
        self.name = ""
        self.outdir = ""
        self.pid = None
        self.eid = None
        self.nsfile = None
        self.eipsfile = None
        self.iipsfile = None
        self.commondirs = {}  # Local/remote src -> remote dst
        self.commonfiles = {} # Local/remote src -> remote dst
        self.duration = -1
        self.preexecution = None
        self.execution = []
        self.postexecution = None
        self.reset = None
        self.parse()

    def dart_getAttribute(self, node, attr):
        return self.sub_variables(node.getAttribute(attr))

    def parse(self):
        data = open(self.dartfile).read()
        data = self.sub_includes(data)
        self.preprocessed = data
        doc = xml.dom.minidom.parseString(data)
        x = doc.getElementsByTagName("variables")
        if x:
            x1 = x[0]
            self.variables = self.parse_variables(x1)
        else:
            self.variables = {}
        x1 = doc.getElementsByTagName("test")[0]
        self.parse_test(x1)
        x1 = doc.getElementsByTagName("topology")[0]
        self.parse_topology(x1)
        x1 = doc.getElementsByTagName("commonfiles")[0]
        self.parse_commonfiles(x1)
        x = doc.getElementsByTagName("preexecution")
        if x: # Optional script
            x1 = x[0]
            self.preexecution = self.parse_script(x1)
        x1 = doc.getElementsByTagName("execution")[0]
        self.duration = int(self.dart_getAttribute(x1, "duration"))
        self.parse_execution(x1)
        x1 = doc.getElementsByTagName("postexecution")[0]
        self.postexecution = self.parse_script(x1)
        x1 = doc.getElementsByTagName("reset")[0]
        self.reset = self.parse_script(x1)

    def parse_test(self, x1):
        x2 = x1.getElementsByTagName("name")[0]
        self.name = self.sub_variables(gettext(x2.childNodes))
        x = x1.getElementsByTagName("outdir")
        if x:
            x2 = x[0]
            self.outdir = self.sub_variables(gettext(x2.childNodes))

    def parse_variables(self, x1):
        variables = {}
        for i in range(len(x1.childNodes)):
            n = x1.childNodes[i]
            if n.nodeType == n.ELEMENT_NODE and n.nodeName == "name":
                name = gettext(n.childNodes)
                m = x1.childNodes[i + 1]
                assert m.nodeName == "value"
                value = gettext(m.childNodes)
                variables[name] = value
                i += 1
        return variables

    def sub_variables(self, s):
        for name in self.variables:
            value = self.variables[name]
            s = re.sub("\$%s\$" % name, value, s)
        return s

    def sub_includes(self, data):
        newdata = data
        while 1:
            m1 = re.search("([ ]*)(<include\s+.*?/>)", newdata)
            if not m1:
                break
            indentlevel = len(m1.group(1))
            match = m1.group(2)
            m2 = re.search("\s+filename=[\"']([^ \"']+)[\"']", match)
            if m2:
                filename = m2.group(1)
                spaces = " " * indentlevel
                lines = open(filename).readlines()
                for i in range(len(lines)):
                    if i == 0:
                        lines[i] = re.sub("^", "  ", lines[i])
                    else:
                        lines[i] = re.sub("^", spaces + "  ", lines[i])
                filedata = "".join(lines)
                filedata = re.sub("[\n]?$", "", filedata)
                newdata = re.sub(match, filedata, newdata)
            else:
                raise "Invalid include statement", match
        return newdata
    
    def parse_topology(self, x1):
        x2 = x1.getElementsByTagName("project")[0]
        self.pid = self.sub_variables(gettext(x2.childNodes))
        x2 = x1.getElementsByTagName("experiment")[0]
        self.eid = self.sub_variables(gettext(x2.childNodes))
        x2 = x1.getElementsByTagName("nsfile")[0]
        self.nsfile = self.sub_variables(gettext(x2.childNodes))
        if not os.path.exists(self.nsfile):
            raise "ns-2 file %s does not exist" % self.nsfile
        x2 = x1.getElementsByTagName("eipsfile")[0]
        self.eipsfile = self.sub_variables(gettext(x2.childNodes))
        x2 = x1.getElementsByTagName("iipsfile")[0]
        self.iipsfile = self.sub_variables(gettext(x2.childNodes))

    def parse_commonfiles(self, x1):
        for x2 in x1.getElementsByTagName("dir"):
            x3 = x2.getElementsByTagName("src")[0]
            src = self.sub_variables(gettext(x3.childNodes))
            type = self.xml_read_type(x3)
            x3 = x2.getElementsByTagName("dst")[0]
            dst = self.sub_variables(self.dart_dir_sub(gettext(x3.childNodes)))
            self.commondirs[src] = { "dst" : dst, "type" : type }
        for x2 in x1.getElementsByTagName("file"):
            x3 = x2.getElementsByTagName("src")[0]
            src = self.sub_variables(gettext(x3.childNodes))
            type = self.xml_read_type(x3)            
            x3 = x2.getElementsByTagName("dst")[0]
            dst = self.sub_variables(self.dart_dir_sub(gettext(x3.childNodes)))
            self.commonfiles[src] = { "dst" : dst, "type" : type }

    def parse_script(self, x1):
        x2 = x1.getElementsByTagName("script")[0]
        script = self.sub_variables(self.dart_dir_sub(gettext(x2.childNodes)))
        return script

    def parse_execution(self, x1):
        # Commands
        self.execution = []
        for x2 in x1.getElementsByTagName("nodegroup"):
            e = self.parse_nodegroup(x2)
            self.execution.append(e)
        # Faults
        self.faults = []
        for x2 in x1.getElementsByTagName("faultgroup"):
            e = self.parse_faultgroup(x2)
            self.faults.append(e)

    def parse_nodegroup(self, x2):
        e = {}
        x3 = x2.getElementsByTagName("nodes")[0]
        e["vnns"] = self.parse_nodes(self.sub_variables(gettext(x3.childNodes)))
        x3 = x2.getElementsByTagName("cmd")[0]
        e["cmd"] = self.dart_dir_sub(self.sub_variables(gettext(x3.childNodes)))
        if x3.hasAttribute("time"):
            e["time"] = int(self.dart_getAttribute(x3, "time"))
        else:
            e["time"] = 0
        return e

    def parse_faultgroup(self, x2):
        e = {}            
        x3 = x2.getElementsByTagName("nodes")[0]
        e["vnns"] = self.parse_nodes(self.sub_variables(gettext(x3.childNodes)))
        nodefaults = x2.getElementsByTagName("nodefault")
        if not nodefaults:
            raise "Must specify a fault of some type"
        if len(nodefaults) > 1:
            raise "Only one fault per faultgroup"
        if nodefaults:
            nodefault = nodefaults[0]
            e["type"] = "node"
            e["subtype"] = self.dart_getAttribute(nodefaults[0], "type")
            e["time"] = int(self.dart_getAttribute(nodefaults[0], "time"))
        return e
            
    def parse_nodes(self, s):
        if s == "*":
            vnns = "*"
        else:
            vnnmap = {}
            for h in s.split(","):
                m = re.search("(\d+)-(\d+)", h)
                if m:
                    i = int(m.group(1))
                    j = int(m.group(2))
                    for k in range(i, j+1):
                        vnnmap[k] = 1
                else:
                    i = int(h)
                    vnnmap[i] = 1
            vnns = vnnmap.keys()
            vnns.sort()
        return vnns

    def xml_read_type(self, x):
        if x.hasAttribute("type"):
            type = self.dart_getAttribute(x, "type")
            if type not in XML_SRC_TYPES:
                raise "Invalid file type"
            return type
        else:
            return XML_SRC_DEFAULT

    def dart_dir_sub(self, path):
        path = re.sub("\$DART_COMMON_DIR", DART_COMMON_DIR, path)
        path = re.sub("\$DART_MY_INPUT_DIR", DART_MY_INPUT_DIR, path)
        path = re.sub("\$DART_MY_OUTPUT_DIR", DART_MY_OUTPUT_DIR, path)
        path = re.sub("\$DART_ALL_OUTPUT_DIR", DART_ALL_OUTPUT_DIR, path)
        return path

class Dart:
    def __init__(self, dartfile, flags):
        self.df = DartFile(dartfile)
        if flags["preprocessed"]:
            print self.df.preprocessed
            sys.exit(0)
        self.verbose = flags["verbose"]
        self.emulab = Emulab(self.df.pid, self.df.eid, self.df.nsfile,
                             self.df.eipsfile, self.df.iipsfile, self.verbose)
        self.eips = []
        self.iips = []
        self.mastereip = None
        self.masteriip = None
        self.slaveeips = []
        self.slaveiips = []
        self.numnodes = -1
        self.master = None

    def map_vnns(self, vnns, type):
        print "FOO: ", vnns, type
        assert(type == EXTERNAL_IPS or type == INTERNAL_IPS)
        if type == EXTERNAL_IPS:
            allips = self.eips
        else:
            allips = self.iips
        ips = []
        if vnns == "*":
            ips = allips
        else:
            for vnn in vnns:
                ips.append(allips[vnn])
        return ips

    def emulab_setup(self):
        self.emulab.mkexp()
        self.emulab.setup()

    def emulab_teardown(self):
        self.emulab.rmexp()

    def emulab_setips(self): # Assumes emulab_setup() in prior execution
        self.eips, self.iips = self.emulab.read_ipfiles()
        self.mastereip = self.eips[0]
        self.masteriip = self.iips[0]
        self.slaveeips = self.eips[1:]
        self.slaveiips = self.iips[1:]
        self.numnodes = len(self.eips)

    def emulab_faults(self):
        import tempfile
        s = StringIO.StringIO()
        for i in range(len(self.df.faults)):
            f = self.df.faults[i]
            eips = self.map_vnns(f["vnns"], EXTERNAL_IPS)
            if f["type"] == "node" and f["subtype"] == "reboot": # Node reboot
                s.write("(sleep %d;" % f["time"])
                s.write("/usr/testbed/bin/node_reboot -w ")
                for ip in eips:
                    m = re.search("^(pc.*?)\.", socket.gethostbyaddr(ip)[0])
                    nodeid = m.group(1) # (e.g., "pc15")
                    s.write("%s " % nodeid)
                s.write("; ) &\n")
        tmp = tempfile.mktemp(".dart")
        self.emulab.xfer_buf_to(s.getvalue(), tmp) # NOTE: no setsid on Emulab!
        self.emulab.rexec_detached("sh %s" % tmp)

    def dart_setup(self):
        self.emulab_setips()        
        self.master = Master(self, verbose=self.verbose)
        self.master.setup()

    def dart_run(self):
        self.emulab_setips()                
        self.master = Master(self, verbose=self.verbose)        
        self.master.run()

    def dart_reset(self):
        self.emulab_setips()        
        self.master = Master(self, verbose=self.verbose)        
        self.master.reset()

def dart_init(useseed=None, seed=None):
    import random
    if useseed:
        random.seed(seed)
    else:
        random.seed(open("/dev/urandom").read(1024))
