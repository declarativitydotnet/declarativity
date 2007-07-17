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
    print "Usage: pout.py [-e -n <num_nodes>] [-d <duration>] [-w <bw_window>] [-l <simple_lookup_output>] output_dir"
    print

def parse_cmdline(argv): 
    global emulab
    emulab = False

    shortopts = "d:l:n:w:e"
    flags = {"duration" : sys.maxint, "num_nodes" : 0, "bw_window" : 4.0, "simple_lookup" : None}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-n": flags["num_nodes"]     = int(v)
        elif o == "-w": flags["bw_window"]     = int(v)
        elif o == "-d": flags["duration"]      = int(v)
        elif o == "-l": flags["simple_lookup"] = v
        elif o == "-e": emulab = True
        else:
            print_usage()
            exit(3)
    return flags, args

def ts2sec(sec, ns):
    return (float(sec) + (float(ns) / 1000000000.0))

def process_node(file, shash, mhash, rhash):
    match_maintenance_lookup = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux.*,\s*
                                 ([0-9]+),\s*                                  # seconds
                                 ([0-9]+)\]\:\s*                               # nanoseconds
                                 \[[0-9]+,\s*\<lookup,\s*                                # token
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address
                                 ([a-f0-9]+),\s*                               # Lookup key
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Destination IP address
                                 ([0-9]+)\s*                                   # Event ID
                             \>\]$\n""", re.VERBOSE)

    match_simple_lookup = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux.*,\s*
                                 ([0-9]+),\s*                                  # seconds
                                 ([0-9]+)\]\:\s*                               # nanoseconds
                                 \[[0-9]+,\s*\<lookup,\s*                                # token
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address
                                 ([a-f0-9]+),\s*                               # Lookup key
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Destination IP address
                                 (simple_lookup\:[0-9\.\:]+)\s*                # Event ID
                             \>\]$\n""", re.VERBOSE)


    match_lookup_result = re.compile(r"""^.*Print\[PrintWatchReceiveBeforeDemux.*,\s*
                                 ([0-9]+),\s*                                  # seconds
                                 ([0-9]+)\]\:\s*                               # nanoseconds
                                 \[[0-9]+,\s*\<lookupResults,\s*                         # token
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address
                                 ([a-f0-9]+),\s*                               # key
                                 ([a-f0-9]+),\s*                               # key
                                 ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Destination IP address
                                 ([a-zA-Z0-9\.\:\_]+)\s*                         # Event ID
                             \>\]$\n""", re.VERBOSE)

    matchbandwidth = re.compile(r"""^Print\[PrintWatchRemoteSend.*,\s*
                                    ([0-9]+),\s*                               # seconds
                                    ([0-9]+)\]\:\s*                            # nanoseconds
                                    \[[0-9]+,\s*\<([a-zA-Z]+),\s*                        # token
                                    .*\>\]$\n""", re.VERBOSE)

    start_t     = 0.0
    bw_window   = float(flags["bw_window"])
    window_bytes = 0.0
    min_bw = sys.maxint
    max_bw = 0.0
    cnt_bw = 0.0
    sum_bw = 0.0
    
    for line in file:
        lookup = []
	result = []
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
        if matchbandwidth.match(line):
            measure = [x for x in matchbandwidth.split(line) if x]

	    # We only want to measure bandwidth for maintenance traffic... 
            # Ignore simple lookups.
            if lookup and shash.has_key(lookup[-1]): continue
            if result and shash.has_key(result[-1]): continue

            ts = ts2sec(measure[0], measure[1])
            if not start_t: start_t = ts
            if not bytes.has_key(measure[2]):
                print "UNKNOWN MESSAGE: ", measure[2]
                sys.exit(3)
            if (ts - start_t) > bw_window:
                # FINALIZE WINDOW HERE
                # postpone this line to the next window, and finalize
                cnt_bw += 1
                bw = window_bytes/bw_window
                if max_bw < bw: max_bw = bw
                if min_bw > bw: min_bw = bw
                sum_bw += bw
                
                # set up next window
                mt_wins = -1 # don't treat the first tumble as an empty window!
                while ((ts - start_t) > bw_window):
                    start_t += bw_window # tumble the window
                    mt_wins += 1
                window_bytes = bytes[measure[2]]
                if (mt_wins > 0):
                    min_bw = 0
                    cnt_bw += mt_wins
            else: # MIDDLE OF WINDOW
                window_bytes += bytes[measure[2]]

    avg_bw = 0
    if cnt_bw > 0 : avg_bw = sum_bw/cnt_bw
    return avg_bw, min_bw, max_bw


def eval_lookups(shash, mhash, rhash):
    sht_fh  = open("./simple_hop_time.dat", 'w')
    sl_fh   = open('./simple_latency.dat', 'w')
    mht_fh  = open('./maintenance_hop_time.dat', 'w')
    ml_fh   = open('./maintenance_latency.dat', 'w')
    sim_start    = float(sys.maxint)
    successful   = 0
    unsuccessful = 0
    start_sec    = 0 
    start_ns     = 0 
    latency      = []
    hop_time     = []

    for event in mhash.keys():
       mlookup = mhash[event]
       mlookup.sort()                                         # sort([type, sec, ns, ...])
       start_sec = start_ns = -1.0 
       for t in mlookup:                                      # For each lookup tuple t
           sec = float(t[0])
           ns  = float(t[1])
           if start_sec < 0.0 or sec < start_sec or (sec == start_sec and ns < start_ns): 
               start_sec = sec 
               start_ns  = ns      
       if not rhash.has_key(event): 
           if (ts2sec(start_sec, start_ns) - sim_start <= flags["duration"]):
               unsuccessful += 1
           continue                                          # Only care about successful lookups
       successful += 1

       hops = len(mlookup) - 1    # THE hop count
       
       if hops: hop_time.append([start_sec, start_ns, hops])
       if ts2sec(start_sec, start_ns) < sim_start: sim_start = ts2sec(start_sec, start_ns)

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
        try:
            print >> mht_fh, "%s %s" % ((ts2sec(x[0], x[1])-sim_start), x[2]) 
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
       start_sec = start_ns = -1.0 
       for t in slookup:                                      # For each lookup tuple t
           sec = float(t[0])
           ns  = float(t[1])
           if start_sec < 0.0 or sec < start_sec or (sec == start_sec and ns < start_ns): 
               start_sec = sec 
               start_ns  = ns      
       if not rhash.has_key(event): 
           if (ts2sec(start_sec, start_ns) - sim_start <= flags["duration"]):
               unsuccessful += 1
           continue                                          # Only care about successful lookups
       successful += 1

       hops = hops = len(slookup) - 1    # THE hop count
       
       if hops: hop_time.append([start_sec, start_ns, hops])
       if ts2sec(start_sec, start_ns) < sim_start: sim_start = ts2sec(start_sec, start_ns)

       r_sec = rhash[event][0]
       r_ns  = rhash[event][1]
       if hops: latency.append([(ts2sec(r_sec, r_ns) - ts2sec(start_sec, start_ns)), hops])
       ElatVal = ts2sec(r_sec, r_ns) - ts2sec(start_sec, start_ns);
       #if (latVal > 10): print rhash[event], ts2sec(r_sec, r_ns), ts2sec(start_sec, start_ns)

    if latency:
        latency.sort()
        for lat in latency: print >> sl_fh, "%f %d" % (float(lat[0]), int(lat[1]))

    print "Successful simple lookup count: ", successful
    print "Unsuccessful simple lookup count: ", unsuccessful

    hop_time.sort()
    for x in hop_time: 
	try:
       	    print >> sht_fh, "%s %s %s" % ((ts2sec(x[0], x[1])-sim_start), x[2]) 
	except: pass
    sht_fh.close()
    sl_fh.close()    

if __name__ == "__main__":
    global simple_lookup
    global ip_map
    global bytes
    global resultsHash

    resultsHash = {}
    bytes = {"startJoin" : 136, "lookup" : 132, "lookupResults" : 164, "stabilizeRequest" : 136,
             "notifyPredecessor" : 108, "sendPredecessor" : 140, "returnSuccessor" : 140,
             "sendSuccessors" : 140, "pingReq" : 100, "pingResp" : 100}

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

    node_bw = open('./node_bw.dat', 'w')
    nodeid  = 0

    avg_bw = 0.0
    max_bw = 0.0
    min_bw = sys.maxint
    
    avg_counter = 0
    for file in files:
        nodeid = nodeid + 1
        if not os.path.exists(file):
            print "ERROR: file does not exist ", file
            exit(3)
        fh = open(file, "r")

        avg, min, max = process_node(fh, shash, mhash, rhash)
	if avg != 0:
            print >> node_bw, "%d %f %f %f" % (nodeid, float(avg), float(min), float(max))
            if max_bw < max_bw: max_max_bw = max_bw
            if min_bw > min_bw: min_min_bw = min_bw
            avg_bw += avg
            avg_counter += 1
        node_bw.flush()
        fh.close()

    if avg_counter != 0: avg_bw /= avg_counter
    else: avg_bw = -1
    print >> node_bw, "#%d %f %f %f" % (avg_counter, float(avg_bw), float(min_bw), float(max_bw))
    node_bw.close()

    eval_lookups(shash, mhash, rhash)
