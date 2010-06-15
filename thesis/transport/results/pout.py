#!/usr/bin/env python
#
# DESCRIPTION: Setup and run n chord nodes.
#
#
import getopt
import os
import sys

def parse_cmdline(argv): 
    global emulab
    emulab = False

    shortopts = "o:"
    flags = {"output" : 0}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-o": flags["output"]        = int(v)
        else:
            exit(3)
    return flags, args

if __name__ == "__main__":
    hmap = {}
    filename = ""
    max_time = 0;
    try:
        flags, args = parse_cmdline(sys.argv)
        filename = args[0]
    except:
        print "EXCEPTION"
        sys.exit(3)

    file = open(filename, 'r')
    latency    = []
    hop_counts = {} 
    retries    = {}

    meas = []
    for line in file:
	meas = map(float, line.strip().split(' ')[2:5])
	if not meas: continue
	l = float(meas[0])
        h = float(meas[1])
        r = float(meas[2]) - h
	latency.append(l)
	hop_counts[h] = hop_counts.get(h, 0.) + 1.
	retries[r]    = retries.get(r, 0.) + 1.
	if not hmap.has_key(h): hmap[h] = {}

	if hmap[h].has_key(r):
		hmap[h][r].append(l)
        else: hmap[h][r] = [l]

    if flags["output"] == 1:
    	for hop in hmap.keys():
		print "HOP ", hop
		for retry in hmap[hop].keys():
			sum = 0.
			for lat in hmap[hop][retry]: sum += lat
			avg = sum / float(len(hmap[hop][retry]))
			print "\tRETRY %f, AVG LATENCY %f" % (retry, avg) 

    total = float(len(latency))
    if flags["output"] == 2:
    	latency.sort()
    	sum   = 0.
    	for i in range(int(total)):
		sum += 1.
    		if i+1 < int(total) and latency[i] == latency[i+1]: continue
		print "%f %f" % (latency[i], sum/total)
    
    if flags["output"] == 3:
    	print "\nHOP COUNT CDF"
    	hkeys = hop_counts.keys()
    	hkeys.sort()
	sum = 0.
    	for k in hkeys:
		sum += hop_counts[k]
		print "%d %f" % (k, sum/total)
	
    if flags["output"] == 4:
    	print "\nRETRY ATTEMPT CDF"
    	rkeys = retries.keys()
    	rkeys.sort()
	sum = 0.
    	for k in rkeys:
		sum += retries[k]
		print "%d %f" % (k, sum/total)
