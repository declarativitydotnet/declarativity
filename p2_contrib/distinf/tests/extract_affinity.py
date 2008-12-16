#! /usr/bin/env python

# run the script:

import sys, re, os, math, getopt
import time, datetime
from Numeric import *

def parse_line(line, splitter):
    temp = re.split(splitter, line)
    items = re.split(',', temp[1])
    i=0
    for item in items:
        item = item.strip()
        item = item.replace('[', '')
        item = item.replace(']', '')
        item = item.replace('(', '')
        item = item.replace(')', '')
        items[i] = item
        i+=1
    return items

def usage():
  print "Usage: extract-affinity.py   <filename base> <number of nodes> <variables per node> \n"
  sys.exit(1)

def die(msg):
  print "extract-affinity.py: " + msg
  sys.exit(1)

optlist, list = getopt.getopt(sys.argv[1:], ':')

if len(list) != 3:
    usage()

base = list[0]
count = int(list[1])
varsPerNode = int(list[2])
totalVars = varsPerNode * count;

tarcmd = "tar -czf "+base+".tar.gz compile.log synchronize.log "

lastA = zeros([totalVars,totalVars], Float)
lastR = zeros([totalVars,totalVars], Float)
lastE = zeros(totalVars, Int)

AAfile = open(base+"-SentAvailabilityAll.txt", "w")
RAfile = open(base+"-SentResponsibilityAll.txt", "w")
Afile = open(base+"-SentAvailability.txt", "w")
Rfile = open(base+"-SentResponsibility.txt", "w")
Efile = open(base+"-Exemplars.txt", "w")
EAfile = open(base+"-ExemplarsAll.txt", "w")

for i in range(1, count+1):
    filein = str(i) + ".log"
    for line in open(filein):
        if re.search('InsertEvent:',line) and re.search('sentAvailability\(', line):
            items = parse_line(line, 'sentAvailability\(')
            AAfile.write(items[1] + ',' + items[2] + ',' + items[3] + ',' + items[4] + '\n')
            lastA[int(items[2])-1][int(items[1])-1] = float(items[3])
        if re.search('InsertEvent:',line) and re.search('sentResponsibility\(', line):
            items = parse_line(line, 'sentResponsibility\(')
            RAfile.write(items[1] + ',' + items[2] + ',' + items[3] + ',' + items[4] + '\n')
            lastR[int(items[1])-1][int(items[2])-1] = float(items[3])
        if re.search('SendAction:',line) and re.search('maxExemplar\(', line):
            items = parse_line(line, 'maxExemplar\(')
            EAfile.write(items[0] + ',' + items[1] + ',' + items[2] +'\n')
            lastE[int(items[1])-1] = int(items[2])

    # incrementally build the cmd to tar-gzip all the log files
    tarcmd = tarcmd + filein + " "

for i in range(0, totalVars):
    for k in range(0, totalVars):
        Afile.write(str(i+1) + ',' + str(k+1) + ',' + str(lastA[i][k]) + '\n')
for i in range(0, totalVars):
    for k in range(0, totalVars):
        Rfile.write(str(i+1) + ',' + str(k+1) + ',' + str(lastR[i][k]) + '\n')
for i in range(0, totalVars):
    Efile.write(str(i+1) + ',' + str(lastE[i]) + '\n')

AAfile.close()
RAfile.close()
Afile.close()
Rfile.close()
Efile.close()
EAfile.close()

print tarcmd
os.system(tarcmd)
