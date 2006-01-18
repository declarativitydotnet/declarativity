#!/usr/bin/env python2
# -*- Mode: python -*-
#
# DESCRIPTION: Setup and run n chord nodes.
#
#
import getopt
import os
import sys
import re

def ts2sec(sec, ns):
    return (float(sec) + (float(ns) / 1000000000.0))

def print_usage():
    print
    print "Usage: plookups.py dir"
    print

def process_node(file, shash, mhash, rhash):
    match_maintenance_lookup = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux.*,\s*
                                 ([0-9]+),\s*                                  # seconds
                                 ([0-9]+)\]\:\s*                               # nanoseconds
                                 \[\<lookup,\s*                                # token
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address
                                 ([a-f0-9]+),\s*                               # Lookup key 
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Destination IP address
                                 ([0-9]+)\s*                                   # Event ID
                             \>\]$\n""", re.VERBOSE)

    match_simple_lookup = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux.*,\s*
                                 ([0-9]+),\s*                                  # seconds
                                 ([0-9]+)\]\:\s*                               # nanoseconds
                                 \[\<lookup,\s*                                # token
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address
                                 ([a-f0-9]+),\s*                               # Lookup key 
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Destination IP address
                                 (test\:[0-9]+)\s*                             # Event ID
                             \>\]$\n""", re.VERBOSE)


    match_lookup_result = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux.*,\s*
                                 ([0-9]+),\s*                                  # seconds
                                 ([0-9]+)\]\:\s*                               # nanoseconds
                                 \[[0-9]+,\s*\<lookupResults,\s*               # token
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address
                                 ([a-f0-9]+),\s*                               # key 
                                 ([a-f0-9]+),\s*                               # key 
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Destination IP address
                                 ([a-zA-Z0-9\.\:]+)\s*                         # Event ID
                             \>\]$\n""", re.VERBOSE)

    for line in file: 
        if match_maintenance_lookup.match(line):
            lookup      = [x for x in match_maintenance_lookup.split(line) if x]
            key         = lookup[-1]
            lookup_list = [lookup]    
            if mhash.has_key(key): lookup_list += mhash[key]
            mhash[key] = lookup_list 
        if match_simple_lookup.match(line):
            lookup      = [x for x in match_simple_lookup.split(line) if x]
            key         = lookup[-1]
            lookup_list = [lookup]    
            if shash.has_key(key): lookup_list += shash[key]
            shash[key] = lookup_list 
        if match_lookup_result.match(line):
            result        = [x for x in match_lookup_result.split(line) if x]
            key           = result[-1]                                 # eventID
            rhash[key]    = result[:-1]


def eval_lookups(shash, mhash, rhash):
    sht_fh  = open("./simple_hop_time.dat", 'w')
    sl_fh   = open('./simple_latency.dat', 'w')
    mht_fh  = open('./maintenance_hop_time.dat', 'w')
    ml_fh   = open('./maintenance_latency.dat', 'w')
    successful   = 0
    unsuccessful = 0
    start_sec    = 0 
    start_ns     = 0 
    latency      = []
    hop_time     = []

    for event in mhash.keys():
       if not rhash.has_key(event): 
           unsuccessful += 1
           continue                                          # Only care about successful lookups
       successful += 1
       mlookup = mhash[event]
       mlookup.sort()                                         # sort([type, sec, ns, ...])
       start_sec = start_ns = -1.0 
       for t in mlookup:                                      # For each lookup tuple t
           sec = float(t[0])
           ns  = float(t[1])
           if start_sec < 0.0 or sec < start_sec or (sec == start_sec and ns < start_ns): 
               start_sec = sec 
               start_ns  = ns      

       hops = len(mlookup) - 1    # THE hop count
       
       if hops: hop_time.append([start_sec, start_ns, hops])

       r_sec = rhash[event][0]
       r_ns  = rhash[event][1]
       if hops: latency.append([(ts2sec(r_sec, r_ns) - ts2sec(start_sec, start_ns)), hops])

    if latency:
        latency.sort()
        for lat in latency: print >> ml_fh, "%f %d" % (float(lat[0]), int(lat[1]))
    print "Successful maintenance lookup count: ", successful
    print "Unsuccessful maintenance lookup count: ", unsuccessful

    hop_time.sort()
    for x in hop_time: 
        print >> mht_fh, "%s %s %s" % (x[0], x[1], x[2]) 
    mht_fh.close()
    ml_fh.close()

    successful   = 0
    unsuccessful = 0
    latency      = []
    hop_time     = []
    for event in shash.keys():
       if not rhash.has_key(event): 
           unsuccessful += 1
           continue                                          # Only care about successful lookups
       successful += 1
       slookup = shash[event]
       slookup.sort()                                         # sort([type, sec, ns, ...])
       start_sec = start_ns = -1.0 
       for t in slookup:                                      # For each lookup tuple t
           sec = float(t[0])
           ns  = float(t[1])
           if start_sec < 0.0 or sec < start_sec or (sec == start_sec and ns < start_ns): 
               start_sec = sec 
               start_ns  = ns      

       hops = hops = len(slookup) - 1    # THE hop count
       
       if hops: hop_time.append([start_sec, start_ns, hops])

       r_sec = rhash[event][0]
       r_ns  = rhash[event][1]
       if hops: latency.append([(ts2sec(r_sec, r_ns) - ts2sec(start_sec, start_ns)), hops])

    if latency:
        latency.sort()
        for lat in latency: print >> sl_fh, "%f %d" % (float(lat[0]), int(lat[1]))
    print "Successful simple lookup count: ", successful
    print "Unsuccessful simple lookup count: ", unsuccessful

    hop_time.sort()
    for x in hop_time: 
        print >> sht_fh, "%s %s %s" % (x[0], x[1], x[2]) 
    sht_fh.close()
    sl_fh.close()


if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) < 1:
        print_usage()        
        sys.exit(3)

    files = filter(lambda files: files[-4:] == '.log', os.listdir(args[0]) )

    shash = {}
    mhash = {}
    rhash = {}

    for file in files:
        file = args[0] + "/" + file;
        if not os.path.exists(file):
            print "ERROR: file does not exist ", file
            exit(3)
        fh = open(file, "r")
        process_node(fh, shash, mhash, rhash)

    eval_lookups(shash, mhash, rhash)
