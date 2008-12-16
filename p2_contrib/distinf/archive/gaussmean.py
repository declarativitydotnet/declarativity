#!/usr/bin/python

#Assuming that the print.olg which we get from printgraph.py has the tuple of the mean within it
# format of the node gaussian mean tuple is 
#	##Print[SendAction!nodeMean!mean_eca!localhost:10000]:  [nodeMean(localhost:10000, [0.166667, 0.166667])]
#	##Print[SendAction!varMean!varmean_eca!localhost:10000]:  [varMean(localhost:10000, (1), [0.166667])]

import sys, re, os, string
#print sys.argv

if len(sys.argv) != 4:
	print 'Usage: gaussmean.py <input file> <node means output file> <variable means output file>'
	sys.exit(1)

filein = sys.argv[1]
fileout1 = sys.argv[2]
fileout2 = sys.argv[3]

fileHandle1 = open(fileout1, "w")
fileHandle2 = open(fileout2, "w")

for line in open(filein):
	#print "line: " + line
	p = re.compile("""
		(nodeMean)
		""", re.VERBOSE|re.DOTALL|re.MULTILINE)
	result = p.search(line)  
	if(result):
		line = line.replace("nodeMean", "")
		temp = line.split("[")
		temp[2] = temp[2][(temp[2].find("("))+1:temp[2].find(",")]
		temp[3] = temp[3][0 : temp[3].find("]")-1]	
		mean = temp[3].split(",")
		fileHandle1.write((temp[2][temp[2].find(":")+1:len(temp[2])]).lstrip().rstrip() + " " + mean[0].rstrip().lstrip() + " " +  mean[1].rstrip().lstrip() + "\n")
	
	p = re.compile("""
                (varMean)
                """, re.VERBOSE|re.DOTALL|re.MULTILINE)
        result = p.search(line)
	if(result):
		line = line.replace("varMean", "")
                temp = line.split("[")
		var = temp[2][1:len(temp[2])]
		var = var[var.find("(")+1:var.find(")")]
                node = temp[2][(temp[2].find("("))+1:temp[2].find(",")]
		node = node[node.find(":")+1:len(node)]
		mean = temp[3][temp[3].find("[")+1:temp[3].find("]")]
		if(mean == ""):
			mean = str(0)
		#print node, var, mean
		fileHandle2.write(node + " " + var + " " + mean + "\n")


fileHandle1.close()
fileHandle2.close()
