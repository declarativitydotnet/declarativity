#! /usr/bin/env python
import sys, re, os, math
print sys.argv

if (len(sys.argv) < 3): 
       print 'Usage: printspantree.py algo num_nodes [--skip-olg]' 
       sys.exit(1)

print sys.argv
filein = "temp/"+sys.argv[1]+sys.argv[2]+".out"
fileout = "temp/"+sys.argv[1]+sys.argv[2]+".dot"
linksfile = "../data/temperature-estimation/model_links_localhost_"+sys.argv[2]+".csv"

if (sys.argv[1]=="bp"):
	a = math.sqrt(float(sys.argv[2]))
	astr = str(int(a))
	print astr
	linksfile = "../grid-data/circle"+astr+"x"+astr+"_local_links.csv"	

if (len(sys.argv) == 4 and os.path.exists(filein) and sys.argv[4]=="--skip-olg"):
	print filein+" exists - using it; neato layout file "+fileout+" from links file "+linksfile+"."
else:
	print "Creating overlog output file "+filein+"; neato layout file "+fileout+" from links file "+linksfile+"."

	cmd = "rm " + filein
	print cmd
	os.system(cmd)
	
	runOverlogCmd = "/Users/ashima/courses/CS281/Project/uai/distinf/src_build/bin/runStagedOverlog -p 12345 -o printspantree.olg -DLINKS_FILE=\\\""+linksfile+"\\\" -DNODE_ADDR=\\\"localhost:12345\\\" > " + filein + " 2>&1 &"
	print runOverlogCmd
	result = os.system(runOverlogCmd)
	print "runOverlog result: " + str(result)
	
	size = -1
	while(size <= 0):
		cmd = "sleep 1"
		print "."
		os.system(cmd)
		size = os.path.getsize(filein)		
	print "10 ... 9..."
	cmd = "sleep 10"
	os.system(cmd)
	
	cmd = "kill `ps | grep printspantree.olg | grep -v grep | awk '{print $1}'`"
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


fin = open(filein, 'r')
fout = open(fileout, 'w')

lines = fin.readlines()

fout.write("digraph G {\n")
fout.write("\torientation=landscape\n")
fout.write("\tcenter=true\n")

for line in lines:
	# print "line: "+line
	if (line.startswith("##Print") and re.search('printSpanTreeEdge\(', line)):
		splitter = 'printSpanTreeEdge\('
		boldEdge = 1
	elif (line.startswith("##Print") and re.search('printLink\(', line)):
		splitter = 'printLink\('
		boldEdge = 0
	else:
		continue
	if (splitter == ''): continue
	items = parse_line(line, splitter)
	src = items[1]
	dest = items[2]
	cost = items[3]
	dotLine = "\t\"" + src + "\" -> \"" + dest + "\""
	if (boldEdge):
		dotLine = dotLine + " [style=bold]"
	elif (cost):
		dotLine = dotLine + " [label=\"" +cost+ "\", style=dotted]"
	dotLine = dotLine + ";\n"
	print dotLine
	fout.write(dotLine)

fout.write("}\n")

fin.close()
fout.close()

cmd = "open " + fileout
print cmd
os.system(cmd)
