#! /usr/bin/env python

# run the script:

import sys, re, os
print sys.argv

if len(sys.argv) != 2: 
	   print 'Usage: printgraph.py <overlog output file>' 
	   sys.exit(1)

filein = sys.argv[1]

def parse_line(line, splitter):
#	print "splitter: "+splitter
	temp = re.split(splitter, line)
	items = re.split(',', temp[1])
#	print items
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

varToNode = {}

for line in open(filein):
#	print "line: "+line
	if (re.search('printSpanTreeEdge\(', line)):
		splitter = 'printSpanTreeEdge\('
		items = parse_line(line, splitter)
#		print items
		pair = [items[1], items[2]]
		edgeList.append(pair)
		copy = pair[:]
		copy.sort()
		tpair = tuple(copy)
		edgeSet[tpair] = ''
	elif (re.search('printLink\(', line)):
		splitter = 'printLink\('
		items = parse_line(line, splitter)
#		print items
		pair = [items[1], items[2]]
		linkList.append(pair)
		copy=pair[:]
		copy.sort()
		tpair = tuple(copy)
		linkSet[tpair] = items[3]
	elif (re.search('printVarCarried\(', line)):
		splitter = 'printVarCarried\('
		items = parse_line(line, splitter)
#		print items	 
		if (items[2] in varToNode):
			listOfNodes = varToNode[items[2]]
			listOfNodes.append(items[1])
		else:
			listOfNodes = [0]
			listOfNodes.append(items[1])
			varToNode[items[2]] = listOfNodes
			
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
#		print items	 
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
#		print items
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
#		print "Line didn't match any splitter: "+line
		continue

#items = []
#linkList = []
#edgeList = []
#nodeToCarried = {}
#nodeToClique = {}
#linkSet = {}
#edgeSet = {}
#nodeToSeps = {}
#varToNode = {}

def check(originNode, srcNode, destNode, var, count):
#	if (count > 9):
#		print "max depth 9, quitting"
#		return False;
#	else: 
#		count = count + 1
#	print "checking: "+srcNode+" "+destNode+" "+var
	result = False
	neighbors= []
	for (one, two) in edgeList:
		if (one == srcNode and two != originNode):
			if (one == srcNode and two == destNode):
				return True
			twoClique = nodeToClique[two]
			if (var in twoClique):
				result = check(srcNode, two, destNode, var, count)
				if (result==True): return True
			else:
				result = False
	return result


for node in nodeToCarried:
	listOfVars = nodeToCarried[node]
	listOfVars.remove(0)
	nodeToCarried[node]=listOfVars

for node in nodeToClique:
	listOfVars = nodeToClique[node]
	listOfVars.remove(0)
	nodeToClique[node]=listOfVars

finalAnswer = True

for var in varToNode:
	varToNode[var].remove(0)
	print "now analyzing var "+ str(var) + ": " + str(varToNode[var])
	listOfNodes = varToNode[var]
	if (len(listOfNodes)==1):
		print "var "+var+" is in 1 node only."
		continue
	for i,node in enumerate(listOfNodes):
		for j in range (i, len(listOfNodes)):
			nodeToCheck = listOfNodes[j]
			if (node == nodeToCheck): continue
#			print "checking: "+node+" "+nodeToCheck+" "+var
			result = check('', node, nodeToCheck, var, 0)
			if (result==False): 
				print "OH NO!!!"
				finalAnswer = False
			else: print "YYAAAAHHHHH!!"

print "FINAL ANSWER: " + str(finalAnswer)
