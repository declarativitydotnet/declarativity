#!/usr/bin/env python2
# -*- Mode: python -*-
#
# DESCRIPTION: Setup and run n chord nodes.
#
#
import getopt
import os
import sys
import time
import random
import signal
import threading

class Monitor:
    def __init__(self, sec, pid, log):
        self.pid = pid
        self.log = log
	if sec: threading.Timer(sec, (lambda: self.kill())).start()

    def kill(self):
        try:
            os.kill(self.pid, signal.SIGKILL)
            print >> self.log, "Killed is successful"
        except:
            print >> self.log, sys.exc_info()[:2]
    

def print_usage():
    print
    print "Usage: run_node -i <IP> -p <start_port> [-l <landmark_ip:port> [-n <num_nodes>]] [-s <seed>] [-t <session_time>] main_dir [output_dir]"
    print "DESC:  Start some number of chord nodes on an emulab host."
    print "DESC:  If arg seed is not given then a random seed in [0, 2^32-1] is supplied ."
    print "DESC:  If a landmark node is not supplied then it is assumed to be the master."
    print "DESC:  If a landmark node exists then a number of slaves can be supplied and assigned ports [0-(num_nodes-1)]."
    print

def parse_cmdline(argv): 
    shortopts = "n:i:p:l:t:f:"
    flags = {"num_nodes" : 1, "seed" : random.random()*sys.maxint, "IP" : None, 
             "start_port" : None, "landmark" : None, "session" : 0.0, "nochurn" : -1}
    opts, args = getopt.getopt(argv[1:], shortopts)
    for o, v in opts:
        if   o == "-n": flags["num_nodes"]  = v
        elif o == "-s": flags["seed"]       = v
        elif o == "-i": flags["IP"]         = v
        elif o == "-p": flags["start_port"] = v
        elif o == "-l": flags["landmark"]   = v
        elif o == "-f": flags["nochurn"]    = int(v)
        elif o == "-t": flags["session"]    = float(v)
    return flags, args

# e.g., runChord <loggingLevel> <seed> <myipaddr:port> [<landmark_ipaddr:port>]\n";
def run_node(log, main_dir, seed, ip, p, lm, out):
    print >> log, "NODEIP = %s:%s" % (ip, p)
    print >> log, "LANDMARK = %s"  % (lm)
    print >> log, "%s/runChord %s/chord2.plg NONE %d %s:%s 10 %s >> %s 2>&1" \
                  % (main_dir, main_dir, seed, ip, p, lm, out) 

    pid = os.fork()
    if pid == 0:
        if lm: rv = os.system(r"%s/runChord %s/chord2.plg NONE %d %s:%s 10 %s >> %s 2>&1" \
                              % (main_dir, main_dir, seed, ip, p, lm, out))
        else:  rv = os.system(r"%s/runChord %s/chord2.plg NONE %d %s:%s 0 >> %s 2>&1" \
                              % (main_dir, main_dir, seed, ip, p, out))
        print >> log, "SYSTEM CALL EXIT"
        sys.exit(0)
    return pid

if __name__ == "__main__":
    try:
        flags, args = parse_cmdline(sys.argv)
    except:
        print_usage()
        sys.exit(3)
    if len(args) < 1:
	print "HERE"
        print_usage()        
        sys.exit(3)

    log = open(args[1]+"/setup_node.log", 'w')

    for node in range(int(flags["num_nodes"])):
        try:
            os.remove(args[1]+"/chord_node" + str(node) + ".out")
        except: pass 

    seeds  = [int(flags["seed"]) + x for x in range(int(flags["num_nodes"]))]
    start  = time.time()
    procs  = [] 
    nodes  = int(flags["num_nodes"])
    port   = int(flags["start_port"])

    for node in range(nodes):
        pid = run_node(log, args[0], seeds[node], flags["IP"], port, flags["landmark"], \
                       args[1]+"/chord_node" + str(node) + ".out")
        port += 1
        print >> log, "NODE %d START TIME %d\n" % (node, time.time() - start)
        if flags["session"] and node != flags["nochurn"]: 
            procs.append(Monitor(450.0 + (random.random() * flags["session"]), pid, log))
        else: procs.append(Monitor(0, pid, log))

    while procs:
        log.flush()
        p, s = os.wait() 
        os.system(r"ps -ef | awk '{print $2}' | grep %d >> %s 2>&1" % (p, args[1]+"/setup_node.log"))
        for node in range(nodes):
            if p == procs[node].pid:
                pid = run_node(log, args[0], seeds[node] + port, flags["IP"], port, flags["landmark"], \
                               args[1]+"/chord_node" + str(node) + ".out")
                procs[node] = Monitor(flags["session"], pid, log) 
                port += 1
                print >> log, "NODE %d RESTART TIME %d\n" % (node, time.time() - start)

