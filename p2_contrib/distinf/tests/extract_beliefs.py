#! /usr/bin/env python

# run the script:

import sys, re, os, math
import time, datetime
print sys.argv

def parse_line(line, splitter):
    #print "splitter: "+splitter
    temp = re.split(splitter, line)
    items = re.split(',', temp[1])
    #print items
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

# def datenum(datestr):
#     items = re.split("\.", datestr)
#     if len(items) == 1:
#         items.append("0")
#     #print items
#     d = time.strptime(items[0],'%Y-%b-%d %H:%M:%S')
#     d0 = datetime.datetime(2000, 1, 1, 0, 0, 0)
#     diff = datetime.datetime(d[0],d[1],d[2],d[3],d[4],d[5])-d0
#     return 730486+diff.days+(diff.seconds+float("0."+items[1]))/86400.0

if len(sys.argv) < 3:
    print 'Usage: extract_beliefs.py <filename base> <number of nodes>'
    sys.exit(1)

base = sys.argv[1]
count = int(sys.argv[2])

id = {}

print "Count = " + str(count)
print "Extracting the ids..."

tarcmd = "tar -czf "+base+".tar.gz compile.log synchronize.log "

# vars for counting the number of messages sent between nodes
maxvarvalue=0
msgs=[]

for i in range(1, count+1):
    #print "Parse log for node " + str(i)
    filein = str(i) + ".log"
    for line in open(filein):
        if re.search('AddAction',line) and re.search('identifier\(', line):
            items = parse_line(line, 'identifier\(')
            id[items[0]] = items[1]

# Messes up jt inference
#         # populate data structures for counting messages
#         if re.search('InsertEvent', line) and re.search('incoming\(', line):
#             items = parse_line(line,'incoming\(')
#             f = int(items[1].strip())
#             t = int(items[2].strip())
#             maxvarvalue=max(f, maxvarvalue)
#             maxvarvalue=max(t, maxvarvalue)
#            msgs.append([f, t])
    # incrementally build the cmd to tar-gzip all the log files
    tarcmd = tarcmd + filein + " "

print "IDs:"
print id
print base
bfile = open(base+"-beliefs.txt", "w")
mfile = open(base+"-msgcounts.txt", "w")
rfile = open(base+"-residuals.txt", "w")
ufile = open(base+"-updates.txt", "w")
nfile = open(base+"-normalizer.txt", "w")
tsfile = open(base+"-tuplesize.txt", "w")
tcfile = open(base+"-tuplecount.txt", "w")

#finish counting messages
print "dim: "+ str(maxvarvalue)
#print msgs
fcounts=[[0]*maxvarvalue for i in range(maxvarvalue)]
tcounts=[[0]*maxvarvalue for i in range(maxvarvalue)]
for m in msgs:
    f=m[0]-1
    t=m[1]-1
    # print str([f,t])
    fcounts[t][f]=fcounts[t][f]+1
    tcounts[f][t]=tcounts[f][t]+1

for i in range(maxvarvalue):
    mfile.write(str(i)+":\t\tFrom\tTo"+'\n')
    for j in range(maxvarvalue):
        if ((fcounts[j][i] == 0) and (tcounts[i][j] == 0)): continue
        mfile.write("\t"+str(j)+":\t"+str(fcounts[j][i])+"\t"+str(tcounts[i][j])+'\n')

# Mark the different types of tuples
tuplealg = { }
tuplealg["load"] = "exp"
tuplealg["config"] = "ignore"
tuplealg["configBroadcast"] = "rst"
tuplealg["possibleEdge"] = "rst"
tuplealg["possibleReachvar"] = "jt"
tuplealg["reachvars"] = "jt"
tuplealg["separator"] = "jt"
tuplealg["message"] = "inference"
tuplealg["incoming"] = "inference"
tuplealg["receivedResidual"] = "agg"
tuplealg["lookup"] = "dht"
tuplealg["lookupResults"] = "dht"
tuplealg["succ"] = "dht"
tuplealg["pingReq"] = "dht"
tuplealg["pingResp"] = "dht"
tuplealg["joinReq"] = "dht"
tuplealg["intermediateResult"] = "dht"
tuplealg["variableLocationEvent"] = "dht"
tuplealg["targetLocation"] = "dht"

algsize = { }
algcount = { }
algs = ["exp", "rst", "jt", "inference", "agg", "dht"]

for alg in algs:
    algsize[alg] = 0
    algcount[alg] = 0
algsize["ignore"] = 0
algcount["ignore"] = 0


print "Extracting the beliefs..."
for i in range(1, count+1):
    for alg in algs:
        algsize[alg] = 0
        algcount[alg] = 0
 
    print "Parsing log for node " + str(i)
    filein = str(i) + ".log"
    for line in open(filein):
        if re.search('SendAction', line) and re.search('beliefValues\(', line):
            items = parse_line(line, 'beliefValues\(')
            bfile.write(items[1]+' '+items[2]+' '+items[3]+' '+items[4]+'\n')
        if re.search('SendAction', line) and re.search('tempMean\(', line):
            items = parse_line(line, 'tempMean\(')
            bfile.write(id[items[0]]+' '+items[1]+' '+items[2]+'\n')
        if re.search('SendAction', line) and re.search('messageUpdates\(', line):
            items = parse_line(line, 'messageUpdates\(')
            ufile.write(id[items[0]]+' '+items[1]+' '+items[2]+'\n')
        if re.search('InsertEvent', line) and re.search('normalizer\(', line):
            items = parse_line(line, 'normalizer\(')
            nfile.write(id[items[0]]+' '+items[1]+' '+items[2]+'\n')
        if re.search('SendAction', line) and re.search('messageResidual\(', line):
            items = parse_line(line, 'messageResidual\(')
            if items[1].strip() == "NULL":
                items[1]="nan"
            rfile.write(id[items[0]]+' '+items[1]+' '+items[2]+'\n')
        if re.search('##TupleCount', line):
            items = re.split(',', line)
            tcfile.write(str(i) + ' ' + items[1].strip() + ' ')
            for j in range(3, len(items)):
                pair = re.split(':', items[j].strip())
                alg=tuplealg[re.split('_',pair[0])[0]]
                algcount[alg] = algcount[alg] + int(pair[1])
            for alg in algs:
                tcfile.write(str(algcount[alg]) + ' ')
            tcfile.write(items[2] + '\n')
        if re.search('##TupleSize', line):
            items = re.split(',', line)
            tsfile.write(str(i) + ' ' + items[1].strip() + ' ')
            for j in range(3, len(items)):
                pair = re.split(':', items[j].strip())
                alg=tuplealg[re.split('_',pair[0])[0]]
                algsize[alg] = algsize[alg] + int(pair[1])
            for alg in algs:
                tsfile.write(str(algsize[alg]) + ' ')
            tsfile.write(items[2] + '\n')

bfile.close()
mfile.close()
rfile.close()
ufile.close()
nfile.close()
tcfile.close()
tsfile.close()

print tarcmd
os.system(tarcmd)
