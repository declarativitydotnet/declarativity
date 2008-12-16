#! /usr/bin/env python

# run the script:
# ./printgraph.py kuang/test10.out kuang/test10.dot data/local10-links-top20.csv
# ./printgraph.py kuang/test4.out kuang/test4.dot local4_links.csv

import sys, re, os
print sys.argv

if len(sys.argv) != 4: 
       print 'Usage: printgraph.py <overlog output file> <graph output> <links file>' 
       sys.exit(1)

filein = sys.argv[1]
fileout = sys.argv[2]
linksfile = sys.argv[3]
print "Creating overlog output file "+filein+"; neato layout file "+fileout+" from links file "+linksfile+"."

# overLog="/home/eecs/kuangc/workspace/p2/trunk-build/tests/runOverLog"
# runOverlogCmd = overLog+" -p 12345 -n r9.millennium.berkeley.edu -o printgraph.olg -DLINKS_FILE=\\\""+linksfile+"\\\" -DNODE_ADDR=\\\"r9.millennium.berkeley.edu:12345\\\" > " + filein + " 2>&1 &"
# 
# print runOverlogCmd
# result=os.system(runOverlogCmd)
# 
# cmd = "sleep 15"
# print cmd
# os.system(cmd)

def parse_line(line, splitter):
    print "splitter: "+splitter
    temp = re.split(splitter, line)
    items = re.split(',', temp[1])
    print items
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

items = []
linkList = []
edgeList = []

nodeToCarried = {}
nodeToClique = {}
linkSet = {}
edgeSet = {}
nodeToSeps = {}

for line in open(filein):
    print "line: "+line
    
    if (re.search('printSpanTreeEdge\(', line)):
        splitter = 'printSpanTreeEdge\('
        items = parse_line(line, splitter)
        print items
        pair = [items[1], items[2]]
        pair.sort()
        tpair = tuple(pair)
        edgeList.append(tpair)
        edgeSet[tpair] = ''
    elif (re.search('printLink\(', line)):
        splitter = 'printLink\('
        items = parse_line(line, splitter)
        print items
        pair = [items[1], items[2]]
        pair.sort()
        tpair = tuple(pair)
        linkList.append(tpair)
        linkSet[tpair] = items[3]
    elif (re.search('printVarCarried\(', line)):
        splitter = 'printVarCarried\('
        items = parse_line(line, splitter)
        print items     
        if (items[1] in nodeToCarried):
            listOfVars = nodeToCarried[items[1]]
            listOfVars.append(items[2])
            nodeToCarried[items[1]] = listOfVars
        else:
            listOfVars = [0]
            listOfVars.append(items[2])
            nodeToCarried[items[1]] = listOfVars
    elif (re.search('printClique\(', line)):
        splitter = 'printClique\('
        items = parse_line(line, splitter)
        print items     
        if (items[1] in nodeToClique):
            listOfVars = nodeToClique[items[1]]
            listOfVars.append(items[2])
            nodeToClique[items[1]] = listOfVars
        else:
            listOfVars = [0]
            listOfVars.append(items[2])
            nodeToClique[items[1]] = listOfVars
    elif (re.search('printSeparator\(', line)):
        splitter = 'printSeparator\('
        items = parse_line(line, splitter)
        print items
        pair = [items[1], items[2]]
        pair.sort()
        tpair = tuple(pair)
        if (tpair in nodeToSeps):
            listSeps = nodeToSeps[tpair]
            if (items[3] not in listSeps):
                listSeps.append(items[3])
            nodeToSeps[tpair] = listSeps
        else:
            nodeToSeps[tpair] = list(items[3])
    else:
        print "Line didn't match printSpanTreeEdge or printLink: "+line
        continue

fout = open(fileout, 'w')
fout.write("graph G {\n")
fout.write("\torientation=landscape\n")
fout.write("\tcenter=true\n")
fout.write("\toverlap=scale\n")

for node in nodeToCarried:
    listOfVars = nodeToCarried[node]
    listOfVars.remove(0)
    nodeToCarried[node]=listOfVars

for node in nodeToClique:
    listOfVars = nodeToClique[node]
    listOfVars.remove(0)
    nodeToClique[node]=listOfVars

for node in nodeToCarried:
    dotLine = "\t" + "\"" + node + "\" [ label=\"" + node + ": carries "+ str(nodeToCarried[node]) + ""
    if (node in nodeToClique):
        dotLine = dotLine + "\nclique " + str(nodeToClique[node])
    dotLine = dotLine + "\" ];\n"
    print dotLine
    dotLine = dotLine.replace('\'', '')
    fout.write(dotLine)
for (src,dst) in linkSet:
    dotLine = "\t\"" + src + "\" -- \"" + dst + "\""
    dotLine = dotLine + " ["
    if ((src,dst) in edgeSet):
        dotLine = dotLine + " style=bold"
        if ((src,dst) in nodeToSeps):
            dotLine = dotLine + " label=\"" + str(nodeToSeps[(src,dst)]) + "\""
    else:
        dotLine = dotLine + " style=dotted"
    dotLine = dotLine + " ];\n"
    print dotLine
    dotLine = dotLine.replace('\'', '')
    fout.write(dotLine)

fout.write("}\n")

fout.close()

print "all done!"

#cmd = "open " + fileout
#print cmd
#os.system(cmd)

cmd = "kill `ps ux | grep printgraph.olg | grep -v grep | awk '{print $2}'`"
print cmd
os.system(cmd)



