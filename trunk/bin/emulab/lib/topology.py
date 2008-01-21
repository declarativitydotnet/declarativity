import os, random, sys, StringIO

# Router parameters
INT_BW        = 100
INT_LAT       = 50

T3_BW         = 45
T3_LAT        = 2

T1_BW         = 1.544
T1_LAT        = 2

# Host parameters
ETH100_BW     = 100
ETH100_LAT    = 2

ETH10_BW      = 10
ETH10_LAT     = 2

CABLE_UP_BW   = 0.384
CABLE_DOWN_BW = 3.000
CABLE_LAT     = 10

DSL_UP_BW     = 0.128
DSL_DOWN_BW   = 0.640
DSL_LAT       = 10

MODEM_BW      = 0.0567
MODEM_LAT     = 50

# Placeholders for LANs
PLACEHOLDER_BW   = 100
PLACEHOLDER_LAT  = 2

# Virtual node constants
DEFAULT_MAX_VMS  = 5

class RouterParams:
    def __init__(self, intbw, intlat, extbw, extlat, intloss=0, extloss=0):
        self.intbw = intbw
        self.intlat = intlat
        self.intloss = intloss
        self.extbw = extbw
        self.extlat = extlat
        self.extloss = extloss
        
class HostParams:
    def __init__(self, upbw, downbw, lat, loss=0.0):
        self.upbw = upbw
        self.downbw = downbw
        self.lat = lat
        self.loss = loss

class Topology:
    def __init__(self, nrouters, nhosts, rparams, hparams, params):
        """
        Simple two-level topology.  Routers in the ns-2 file have
        names like 'router0013'.  Hosts in the ns-2 file have names
        like 'node0003'.  Both router and host numbers start from 0.
        """
        self.routers = range(0, nrouters)
        self.hosts = range(0, nhosts)
        self.rparams = rparams
        self.hparams = hparams
        self.hostlans = {}
        self.lanno = 0
        self.os = params["os"]
        self.rpms = params["rpms"]
        self.script = params["script"]
        self.make_graph(nrouters, nhosts, rparams, hparams)

    def make_graph(self, nrouters, nhosts, rparams, hparams):
        if nrouters >= 2:
            for r in self.routers:
                self.hostlans[r] = []
            for h in self.hosts:
                r = random.randint(0, len(self.routers) - 1)
                self.hostlans[r].append(h)
        else: pass # else star topology with all hosts

    def get_ns2_router_network(self, s):
        nodes = self.routers
        s.write("set lan%04d [$ns make-lan \"" % self.lanno)
        s.write("%s" % (" ".join(map(lambda(x): "$router%04d" % x, nodes))))
        s.write("\" %.3fMb %.3fms]\n" % (PLACEHOLDER_BW, PLACEHOLDER_LAT))
	# END NODE SHAPING
        # s.write("tb-set-endnodeshaping lan%04d 1\n" % self.lanno)
        for r in nodes:
            p = self.rparams[r]
            s.write("tb-set-node-lan-params $router%04d $lan%04d %.3fms %.3fMb %.3f\n" % (r, self.lanno, p.intlat, p.intbw, p.intloss))
        self.lanno += 1
        s.write("\n")

    def get_ns2_host_lans(self, s):
        for r in self.hostlans:
            if len(self.hostlans) == 0:
                continue
            rstr = "$router%04d" % r
            hstr = " ".join(map(lambda(x): "$node%04d" % x, self.hostlans[r]))
            rp = self.rparams[r]
            s.write("set lan%04d [$ns make-lan \"" % self.lanno)
            s.write("%s" % (rstr + " " + hstr))
            s.write("\" %.3fMb %.3fms]\n" % (PLACEHOLDER_BW, PLACEHOLDER_LAT))
	    # END NODE SHAPING
            # s.write("tb-set-endnodeshaping lan%04d 1\n" % self.lanno)
            s.write("tb-set-node-lan-params $router%04d $lan%04d %.3fms %.3fMb %.3f\n" % (r, self.lanno, rp.extlat, rp.extbw, rp.extloss))
            for h in self.hostlans[r]:
                hp = self.hparams[h]
                s.write("tb-set-lan-simplex-params $lan%04d $node%04d %.3fms %.3fMb %.3f %.3fms %.3fMb %.3f\n" % (self.lanno, h, hp.lat, hp.upbw, hp.loss, hp.lat, hp.downbw, hp.loss))
            self.lanno += 1
            s.write("\n")

    def get_ns2_host_star(self, s):
        s.write("set lan%04d [$ns make-lan \"" % self.lanno)
        s.write("%s" % " ".join(map(lambda(x): "$node%04d" % x, self.hosts)))
        s.write("\" %.3fMb %.3fms]\n" % (PLACEHOLDER_BW, PLACEHOLDER_LAT))
        for h in self.hosts:
            hp = self.hparams[h]
            s.write("tb-set-lan-simplex-params $lan%04d $node%04d %.3fms %.3fMb %.3f %.3fms %.3fMb %.3f\n" % (self.lanno, h, hp.lat, hp.upbw, hp.loss, hp.lat, hp.downbw, hp.loss))
        self.lanno += 1
        s.write("\n")

    def get_ns2_nodes(self):
        """Return Emulab ns-2 snippet for routers/hosts"""
        s = StringIO.StringIO()
        if len(self.routers) >= 2:
            for r in self.routers:
                s.write("set router%04d [$ns node]\n" % r)
            s.write("\n")
        for h in self.hosts:
            s.write("set node%04d [$ns node]\n" % h)
        return s.getvalue()

    def get_ns2_network(self):
        """Return Emulab ns-2 snippet for network topology"""
        s = StringIO.StringIO()
        if len(self.routers) >= 2:
            self.get_ns2_router_network(s)
            self.get_ns2_host_lans(s)
        else:
            self.get_ns2_host_star(s)
        return s.getvalue()

    def get_ns2_os(self, os="FC6-UPDATE"):
        """Return Emulab ns-2 snippet for setting host operating system"""
        s = StringIO.StringIO()
        for h in self.hosts:
            s.write("tb-set-node-os $node%04d %s\n" % (h, os))
        return s.getvalue()

    def get_ns2_script(self):
        s = StringIO.StringIO()
        if not self.script:
            return None
        for h in self.hosts:
            s.write("tb-set-node-startcmd $node%04d %s\n" % (h, self.script))
        return s.getvalue()

    def get_ns2_rpms(self):
        s = StringIO.StringIO()
        if len(self.rpms) == 0:
            rpmstr = "/proj/P2/rpms/p2-fc6-update.rpm"
        else:
            rpmstr = " ".join(self.rpms)
        for h in self.hosts:
            s.write("tb-set-node-rpms $node%04d %s\n" % (h, rpmstr))
        return s.getvalue()

    def get_ns2(self):
        """Return Emulab ns-2 file to create topology"""
        s = StringIO.StringIO()
        s.write("set ns [new Simulator]\n")
        s.write("source tb_compat.tcl\n\n")
        s.write("%s\n" % self.get_ns2_nodes())
        s.write("%s" % self.get_ns2_network())
        s.write("%s\n" % self.get_ns2_os())
        s.write("%s\n" % self.get_ns2_rpms())
        script = self.get_ns2_script()
        if script:
            s.write("%s\n" % script)
        s.write("tb-set-sync-server $node0000\n\n")
        s.write("$ns rtproto Static\n")
        s.write("$ns run\n")
        return s.getvalue()

class RandomTopology(Topology):
    def __init__(self, nrouters, nhosts, linkprobs, hostprobs, params):
        rparams = []
        hparams = []
        for r in range(nrouters):
            p = random.random()
            q = 0
            for i in range(len(linkprobs)):
                pr, bw, lat = linkprobs[i]
                if i == len(linkprobs) - 1: # Last/only choice left
                    rparams.append(RouterParams(INT_BW, INT_LAT, bw, lat))
                    break
                q += pr
                if p <= q:
                    rparams.append(RouterParams(INT_BW, INT_LAT, bw, lat))
                    break
        for h in range(nhosts):
            p = random.random()
            q = 0
            for i in range(len(hostprobs)):
                pr, upbw, downbw, lat = hostprobs[i]
                if i == len(hostprobs) - 1: # Last/only choice left
                    hparams.append(HostParams(upbw, downbw, lat))
                    break                    
                q += pr
                if p <= q:                
                    hparams.append(HostParams(upbw, downbw, lat))
                    break
        Topology.__init__(self, nrouters, nhosts, rparams, hparams, params)
