#!/usr/bin/env python
# -*- Mode: python -*-
#
# Usage: mkns2 [OPTIONS] nrouters nhosts
#
# DESCRIPTION: Create a parameterized network topology and output an
# Emulab ns-2 suitable for experiment creation on Emulab. 
#
# Example: mkns2 -3 70 -1 30 -c 50 -d 50 6 64
#
# The above creates a topology with 6 routers, 64 hosts such that the
# 64 hosts are connected through 6 access links (70% are T3's, 30% are
# T1's) and hosts are connected via either cable modem or DSL with a
# 50% probability each.
#
#

import getopt
import os
import sys
from topology import *

DEFAULT_LINKPROBS = [ (1.0, T3_BW, T3_LAT) ]
DEFAULT_HOSTPROBS = [ (1.0, ETH10_BW, ETH10_BW, ETH10_LAT) ]

def print_usage():
    print "Usage: mkns2 [OPTIONS] nrouters nhosts"
    print
    print "  -o outfile   Emulab ns-2 file"
    print "  -s os        Emulab operating system"
    print "  -r rpms      P2 rpm path"
    print "  -p prog      Experiment startup program"
    print "  -3 weight    Weight for fraction of link T3 lines"
    print "  -1 weight    Weight for fraction of link T1 lines"
    print "  -e weight    Weight for fraction of 10 mbps Ethernet hosts"
    print "  -c weight    Weight for fraction of cable modem hosts"
    print "  -d weight    Weight for fraction of DSL hosts"
    print

class ns2:
    def __init__(self, argv=None):
        self.argv = argv;
        return

    def apply(self):
        try:
            flags, args = self.parse_cmdline(self.argv)
        except:
            print_usage()
            sys.exit(3)
        if len(args) != 2:
            print_usage()        
            sys.exit(3)
        nrouters = int(args[0])
        nhosts = int(args[1])
        linkprobs = flags["linkprobs"]
        hostprobs = flags["hostprobs"]
        topo = RandomTopology(nrouters, nhosts, linkprobs, hostprobs, flags)
        if flags["outfile"]:
            open(flags["outfile"], "w").write(topo.get_ns2())
            return None
        else:
            return topo.get_ns2()

    def parse_cmdline(self, argv):
        shortopts = "o:3:1:e:c:d:s:r:p:"
        longopts = [ "outfile" ]
        linkweights = {}
        hostweights = {}
        flags = {}
        flags["outfile"] = None
        flags["os"]   = "FC6-UPDATE"
        flags["rpms"] = []
        opts, args = getopt.getopt(self.argv, shortopts, longopts)
        for o, v in opts:
            if o in ("-o", "--outfile"):
                flags["outfile"] = v
            elif o in ("-s", "--os"):
                flags["os"] = v
            elif o in ("-r", "--rpms"):
                flags["rpms"].append(v)
            elif o in ("-p", "--prog"):
                flags["prog"].append(v)
            elif o in ("-3", "--link_t3"):
                linkweights["link_t3"] = float(v)
            elif o in ("-1", "--link_t1"):
                linkweights["link_t1"] = float(v)
            elif o in ("-e", "--host_eth10"):
                hostweights["host_eth10"] = float(v)
            elif o in ("-c", "--host_cable"):
                hostweights["host_cable"] = float(v)
            elif o in ("-d", "--host_dsl"):
                hostweights["host_dsl"] = float(v)
        if len(linkweights) == 0:
            linkprobs = DEFAULT_LINKPROBS
        else:
            linkprobs = self.compute_linkprobs(linkweights)
        flags["linkprobs"] = linkprobs
        if len(hostweights) == 0:
            hostprobs = DEFAULT_HOSTPROBS
        else:
            hostprobs = self.compute_hostprobs(hostweights)
        flags["hostprobs"] = hostprobs        
        return flags, args
    
    def compute_linkprobs(self, linkweights):
        linkprobs = []
        total = 0.0
        probs = {}
        for type in linkweights:
            total += linkweights[type]
        for type in linkweights:
            probs[type] = linkweights[type]/total
            if type == "link_t3":
                linkprobs.append((probs[type], T3_BW, T3_LAT))
            elif type == "link_t1":
                linkprobs.append((probs[type], T1_BW, T1_LAT))
        return linkprobs
                  
    def compute_hostprobs(self, hostweights):
        hostprobs = []
        total = 0.0
        probs = {}
        for type in hostweights:
            total += hostweights[type]
        for type in hostweights:
            probs[type] = hostweights[type]/total
            if type == "host_eth10":
                hostprobs.append((probs[type], ETH10_BW, ETH10_BW, ETH10_LAT))
            elif type == "host_cable":
                hostprobs.append((probs[type], CABLE_UP_BW, CABLE_DOWN_BW,
                                  CABLE_LAT))
            elif type == "host_dsl":
                hostprobs.append((probs[type], DSL_UP_BW, DSL_DOWN_BW, DSL_LAT))
        return hostprobs

if __name__ == "__main__":
    instance = eval("ns2" + "(argv=sys.argv[1:])")
    ns = instance.apply()
    if ns:
      print ns
