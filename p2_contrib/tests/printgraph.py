#! /usr/bin/env python

# run the script:
# ./printgraph.py kuang/test10.out kuang/test10.dot data/local10-links-top20.csv
# ./printgraph.py kuang/test4.out kuang/test4.dot local4_links.csv

import sys, re, os, socket, platform
print sys.argv

if (len(sys.argv) < 4): 
       print 'Usage: \n\t./printgraph.py <algo> <nnodes> <localhost or emulab> [--skip-olg]' 
       sys.exit(1)

filein = "temp/"+sys.argv[1]+sys.argv[2]+".out"
fileout = "temp/"+sys.argv[1]+sys.argv[2]+".dot"

linksfile = "../data/temperature-estimation/model_links_"+sys.argv[3]+"_"+sys.argv[2]+".csv"

hostname = socket.gethostname()

if (len(sys.argv) == 5 and os.path.exists(filein) and sys.argv[4]=="--skip-olg"):
	print filein+" exists - using it; neato layout file "+fileout+" from links file "+linksfile
else:
	print "Creating overlog output file "+filein+"; neato layout file "+fileout+" from links file "+linksfile

	cmd = "rm " + filein
	print cmd
	os.system(cmd)
	
        cmd = "$OVERLOG -n " + hostname + " -p 12345 -o printgraph.olg -DLINKS_FILE=\\\""+linksfile+"\\\" -DNODE_ADDR=\\\""+ hostname +":12345\\\" > " + filein + " 2>&1 &"

	print "cmd: " +cmd
	result = os.system(cmd)
	print "result: " + str(result)
	

	size = -1
	while(size <= 0):
		cmd = "sleep 1"
		print "."
		os.system(cmd)
		size = os.path.getsize(filein)		
        cmd = "sleep " + str(int(sys.argv[2])*3+20)
	print cmd + "..."
	os.system(cmd)
	
	cmd = "kill `ps x | grep printgraph.olg | grep -v grep | awk '{print $1}'`"
	print cmd
	os.system(cmd)

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
    
    if (line.startswith("##Print") and re.search('printSpanTreeEdge\(', line)):
        splitter = 'printSpanTreeEdge\('
        items = parse_line(line, splitter)
        print items
        pair = [items[1], items[2]]
        pair.sort()
        tpair = tuple(pair)
        edgeList.append(tpair)
        edgeSet[tpair] = ''
    elif (line.startswith("##Print") and re.search('printLink\(', line)):
        splitter = 'printLink\('
        items = parse_line(line, splitter)
        print items
        pair = [items[1], items[2]]
        pair.sort()
        tpair = tuple(pair)
        linkList.append(tpair)
        linkSet[tpair] = items[3]
    elif (line.startswith("##Print") and re.search('printVarCarried\(', line)):
        splitter = 'printVarCarried\('
        items = parse_line(line, splitter)
        print items     
        if (items[1] in nodeToCarried):
            listOfVars = nodeToCarried[items[1]]
            listOfVars.append(items[2])
            nodeToCarried[items[1]] = listOfVars
        else:
            nodeToCarried[items[1]] = list(items[2])
    elif (line.startswith("##Print") and re.search('printClique\(', line)):
        splitter = 'printClique\('
        items = parse_line(line, splitter)
        print items     
        if (items[1] in nodeToClique):
            listOfVars = nodeToClique[items[1]]
            listOfVars.append(items[2])
            nodeToClique[items[1]] = listOfVars
        else:
            nodeToClique[items[1]] = list(items[2])
    elif (line.startswith("##Print") and re.search('printSeparator\(', line)):
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
        continue

fout = open(fileout, 'w')
fout.write("graph G {\n")
fout.write("\torientation=landscape\n")
fout.write("\tcenter=true\n")
fout.write("\toverlap=scale\n")

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

s = platform.system()
print s
if (s == "Darwin"):
    cmd = "open " + fileout
    print cmd
    os.system(cmd)
