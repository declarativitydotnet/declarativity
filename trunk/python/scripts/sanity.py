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
    print "Usage: sanity.py event path"
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

def process_file(file, event, event_hash):
    match_event = re.compile(r"""^.*Print\[PrintWatchInsert.*,\s*
                                 [0-9]+\-[A-Za-z]+\-[0-9]+\s                   # Day of year
                                 ([0-9]+):                                     # Hour 
                                 ([0-9]+):                                     # Minute 
                                 ([0-9]+)                                      # seconds 
                                 (\.[0-9]+)\]\:\s*                             # fractional seconds
                                 \[[0-9]+,\s*\<%s,\s*                          # token
                                 ([A-Za-z]+:[0-9]+),\s*                        # Source address
                                 ([a-f0-9]+),\s*                               # Identifier
                                 ([A-Za-z]+:[0-9]+)\s*                        # Destination address
                                  \>\]$\n""" % event, re.VERBOSE)
                                 # ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]+),\s*  # Source IP address

    for line in file:
        if match_event.match(line):
            event      = [x for x in match_event.split(line) if x]
            src        = event[4]
            event_list = [[ts2sec(event[0],event[1],event[2],event[3])] + event[4:]]
            if event_hash.has_key(src): event_list += event_hash[src]
            event_hash[src] = event_list


def eval_lookups(event_hash):
    last_event = open("./last_event.dat", 'w')

    succ_hash = {}
    for src in event_hash.keys():
       event_list = event_hash[src]
       event_list.sort()
       succ = event_list[-1][-1]
       id   = event_list[-1][-2]
       print "%s -> %s (ID: %s)" % (src, succ, id)
       #if succ_hash.has_key(succ):
       #    print "SUCCESSOR ALREADY USED!!"
       #    print "NODE %s HAS BEST SUCC %s" %(src, succ) 
       succ_hash[succ] = True

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
        for node in os.listdir(args[1]):
	    node = args[1] + node
            logs = filter(lambda files: files[-4:] == '.out', os.listdir(node) )
            for l in logs:
                files += [("%s/%s" % (node, l))]
    else:
        logs = filter(lambda files: files[-4:] == '.out', os.listdir(args[1]) )
        for l in logs: files += [("%s%s" % (args[1], l))]
            

    event_hash = {}
    for file in files:
        if not os.path.exists(file):
            print "ERROR: file does not exist ", file
            sys.exit(3)
        fh = open(file, "r")

        process_file(fh, args[0], event_hash)
    eval_lookups(event_hash)
