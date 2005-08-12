import re, socket, popen2, time

class TBReport:
    def __init__(self, project, experiment, nsfile):
        cmd = "ssh users.emulab.net /usr/testbed/bin/expinfo -l -e %s,%s" % (project, experiment)
        child = popen2.Popen3(cmd)
        cstdout = child.fromchild
        self.tblines = cstdout.readlines()
        self.pid = project
        self.eid = experiment
        self.nsfile = nsfile
        self.nodes = self.read_nsnodes(nsfile)
        self.nodes.sort()
        self.eips = self.lookup_external_ips()
        #for i in range(10000): 
        #    print "TRY BLA"
        #    cmd = "ssh users.emulab.net /usr/testbed/bin/expinfo -l -e %s,%s" % (project, experiment)
        #    child = popen2.Popen3(cmd)
        #    cstdout = child.fromchild
        #    self.tblines = cstdout.readlines()
        self.iips = self.read_internal_ips()

    def read_nsnodes(self, nsfile):
        nodes = []
        for l in open(self.nsfile).readlines():
            m = re.search("set\s+(node\d+)\s+\[\s*\$ns\s+node\s*\]", l)
            if m:
                nodes.append(m.group(1))
        return nodes

    def lookup_external_ips(self):
        ips = {}
        for node in self.nodes:
            hostname = "%s.%s.%s.emulab.net" % (node, self.eid, self.pid)
            for retrynum in range(100000):
                try:
                    info = socket.gethostbyname_ex(hostname)
                    break
                except:
                    print "Failed looking up", hostname, "waiting a second"
                    time.sleep(1)
            ip = info[2][0] # Pick first external IP address
            ips[node] = ip
        return ips

    def read_internal_ips(self):
        ips = {}
        for l in self.tblines:
            m = re.search("^(node\d+)\s+node\d+:0\s+(.*?)\s+", l)
            if m:
                node, ip = m.group(1), m.group(2)
                ips[node] = ip
        return ips
