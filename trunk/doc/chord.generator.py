#!/usr/bin/python
## This file is distributed under the terms in the attached LICENSE file.
## If you do not find this file, copies can be found by writing to:
## Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
## Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
## Or
## UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
## Berkeley, CA,  94707. Attention: P2 Group.
## 
## DESCRIPTION: PlanetLab generator for chord.olg

import sys
import os
import getopt
import random
import sha

if __name__ == "__main__":
    overlog = sys.argv[1]
    seed = int(sys.argv[2])
    node = str(sys.argv[3])
    port = str(sys.argv[4])
    outputOverlog = str(sys.argv[5])
    
    shortopts = "D:"

    env = {}
    opts, args = getopt.getopt(sys.argv[6:], shortopts)
    for o, v in opts:
        if   o == "-D": 
            d = v.split("=", 1)
            env[d[0]] = d[1].replace("\"", "\\\"")
    envs = ""
    for var in env:
        envs += "-D" + var + "=" + env[var] + " "
    envs += "-DLOCALADDRESS=\\\"%s:%s\\\" " % (node, port)

    m = sha.new()
    m.update("%s:%s" % (node, port))
    nodeid = m.hexdigest()

    envs += "-DNODEID=0x%sI " % (nodeid)

    command = "cpp -C -P %s %s %s" % (overlog, envs, outputOverlog)
    os.system(command)
    
