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

def print_usage():
    print
    print "Usage: pout.py [-e] [-d <duration>] output_dir"
    print

def parse_cmdline(argv): 
    global emulab
    emulab = False

    shortopts = "d:l:n:w:e"
    flags = {"duration" : sys.maxint}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if o == "-d": flags["duration"]      = int(v)
        elif o == "-e": emulab = True
        else:
            print_usage()
            exit(3)
    return flags, args

def ts2sec(hr, min, sec, fr):
    return (float(hr)*60.*60. + float(min)*60. + float(sec) + float(fr))

def process_file(file, shash, mhash, rhash):
    match_maintenance_lookup = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux,\s*
                                 [0-9]+\-[A-Za-z]+\-[0-9]+\s                   # Day of year
                                 ([0-9]+):                                     # Hour 
                                 ([0-9]+):                                     # Minute 
                                 ([0-9]+)                                      # seconds 
                                 (\.[0-9]+)\]\:\s*                               # fractional seconds
                                 \[[0-9]+,\s*\<lookup,\s*                      # token
                                 ([A-Za-z]+:[0-9]+),\s*                        # Source address
                                 ([a-f0-9]+),\s*                               # Lookup key
                                 ([A-Za-z]+:[0-9]+),\s*                        # Destination address
                                 ([0-9]+)\s*                                   # Event ID
                             \>\]$\n""", re.VERBOSE)
                                 # ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address

    match_lookup = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux,\s*
                                 [0-9]+\-[A-Za-z]+\-[0-9]+\s                   # Day of year
                                 ([0-9]+):                                     # Hour 
                                 ([0-9]+):                                     # Minute 
                                 ([0-9]+)                                      # seconds 
                                 (\.[0-9]+)\]\:\s*                             # fractional seconds
                                 \[[0-9]+,\s*\<lookup,\s*                      # token
                                 ([A-Za-z]+:[0-9]+),\s*                        # Source address
                                 ([a-f0-9]+),\s*                               # Lookup key
                                 ([A-Za-z]+:[0-9]+),\s*                        # Destination address
                                 (lookupGenerator\:[A-Za-z0-9\.\:]+)\s*        # Event ID
                             \>\]$\n""", re.VERBOSE)
                                 # ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address


    match_lookup_result = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux,\s*
                                 [0-9]+\-[A-Za-z]+\-[0-9]+\s                   # Day of year
                                 ([0-9]+):                                     # Hour 
                                 ([0-9]+):                                     # Minute 
                                 ([0-9]+)                                      # seconds 
                                 (\.[0-9]+)\]\:\s*                             # fractional seconds
                                 \[[0-9]+,\s*\<lookupResults,\s*               # token
                                 ([A-Za-z]+:[0-9]+),\s*                        # Source address
                                 ([a-f0-9]+),\s*                               # key
                                 ([a-f0-9]+),\s*                               # key
                                 ([A-Za-z]+:[0-9]+),\s*                        # Destination address
                                 ([a-zA-Z0-9\.\:\_]+)\s*                       # Event ID
                             \>\]$\n""", re.VERBOSE)
                                 # ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address

    for line in file:
        lookup = []
	result = []
        if match_maintenance_lookup.match(line):
            lookup      = [x for x in match_maintenance_lookup.split(line) if x]
            key         = lookup[-1]
            lookup_list = [lookup]
            if mhash.has_key(key): lookup_list += mhash[key]
            mhash[key] = lookup_list
        if match_lookup.match(line):
            lookup      = [x for x in match_lookup.split(line) if x]
            key         = lookup[-1]
            lookup_list = [lookup]
            if shash.has_key(key): lookup_list += shash[key]
            shash[key] = lookup_list
        if match_lookup_result.match(line):
            result        = [x for x in match_lookup_result.split(line) if x]
            key           = result[-1]                                 # eventID
            rhash[key]    = result[:-1]


def eval_lookups(shash, mhash, rhash):
    sht_fh  = open("./lookup_hops.dat", 'w')
    sl_fh   = open('./lookup_latency.dat', 'w')
    mht_fh  = open('./maintenance_hops.dat', 'w')
    ml_fh   = open('./maintenance_latency.dat', 'w')
    sim_start    = float(sys.maxint)
    successful   = 0
    unsuccessful = 0
    time         = 0.
    latency      = []
    hop_time     = []

    for event in mhash.keys():
       mlookup = mhash[event]
       mlookup.sort()                                         # sort([type, sec, ns, ...])
       start_time = -1.0 
       start_tuple = None
       for t in mlookup:                                      # For each lookup tuple t
           time = ts2sec(t[0], t[1], t[2], t[3])
           if start_time < 0.0 or time < start_time: 
               start_time = time
               start_tuple = t
       if not rhash.has_key(event): 
           if (start_time - sim_start <= flags["duration"]):
               unsuccessful += 1
           continue                                          # Only care about successful lookups
       successful += 1

       hops = len(mlookup) - 1    # THE hop count
       
       if hops: hop_time.append([start_time, hops])
       if start_time < sim_start: sim_start = start_time

       response_time = ts2sec(rhash[event][0], rhash[event][1], rhash[event][2], rhash[event][3])
       if hops: latency.append([(response_time - start_time), hops])

    if latency:
        latency.sort()
        for lat in latency: print >> ml_fh, "%f %d" % (float(lat[0]), int(lat[1]))
    print "Successful maintenance lookup count: ", successful
    print "Unsuccessful maintenance lookup count: ", unsuccessful

    hop_time.sort()
    for x in hop_time: 
        try:
            print >> mht_fh, "%s %s" % ((x[0]-sim_start), x[1]) 
        except: pass
    mht_fh.close()
    ml_fh.close()

    successful   = 0
    unsuccessful = 0
    latency      = []
    hop_time     = []
    for event in shash.keys():
       slookup = shash[event]
       slookup.sort()                                         # sort([type, sec, ns, ...])
       start_time = -1.0 
       for t in slookup:                                      # For each lookup tuple t
           time = ts2sec(t[0],t[1],t[2],t[3])
           if start_time < 0.0 or time < start_time: 
               start_time = time
       if not rhash.has_key(event): 
           if (start_time - sim_start <= flags["duration"]):
               unsuccessful += 1
           continue                                          # Only care about successful lookups
       successful += 1

       hops = hops = len(slookup) - 1    # THE hop count
       
       if hops: hop_time.append([start_time, hops])
       if start_time < sim_start: sim_start = start_time


       response_time = ts2sec(rhash[event][0], rhash[event][1], rhash[event][2], rhash[event][3])
       if hops: latency.append([response_time - start_time, hops])

    if latency:
        latency.sort()
        for lat in latency: print >> sl_fh, "%f %d" % (float(lat[0]), int(lat[1]))

    print "Successful simple lookup count: ", successful
    print "Unsuccessful simple lookup count: ", unsuccessful

    hop_time.sort()
    for x in hop_time: 
	try:
       	    print >> sht_fh, "%s %s" % ((x[0]-sim_start), x[1]) 
	except: pass
    sht_fh.close()
    sl_fh.close()    

if __name__ == "__main__":
    try:
        flags, args = parse_cmdline(sys.argv)
    except:
        print "EXCEPTION"
        print_usage()
        sys.exit(3)
    if len(args) < 1:
        print_usage()        
        sys.exit(3)

    files = []
    if emulab:
        for node in os.listdir(args[0]):
	    node = args[0] + node
            logs = filter(lambda files: files[-4:] == '.out', os.listdir(node) )
            for l in logs:
                files += [("%s/%s" % (node, l))]
    else:
        logs = filter(lambda files: files[-4:] == '.out', os.listdir(args[0]) )
        for l in logs: files += [("%s%s" % (args[0], l))]
            

    mhash = {}
    shash = {}
    rhash = {}
    for file in files:
        if not os.path.exists(file):
            print "ERROR: file does not exist ", file
            sys.exit(3)
        fh = open(file, "r")

        process_file(fh, shash, mhash, rhash)
    eval_lookups(shash, mhash, rhash)
