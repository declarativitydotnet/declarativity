#!/usr/bin/env python
import sys, re, os, string
print sys.argv

if len(sys.argv) != 4: 
       print 'Usage: rl_convert.py <input file> <output file> <l or v>' 
       sys.exit(1)

filein = sys.argv[1]
fileout = sys.argv[2]
flag = sys.argv[3]

rldict = {}

hostnum = 9
portbase = 23000
localport = 10001
for i in range(55):
	local = "localhost:"+str(localport+i)
	host = "r" + str(hostnum) + ".millennium.berkeley.edu"
	port = ":"+str(portbase)
	rl = host+port
	print local + "-->" + rl
	rldict[local] = rl
	hostnum = hostnum + 1
	if (hostnum == 35):
		hostnum = 9
	portbase = portbase + 1
	# if (portbase == 23004):
	# 	portbase = 23001
		
fin = open(filein, "r")
fout = open(fileout, 'w')

oneTwo = {}
twoOne = {}

uniqueraw = {}

for line in fin:
	items = line.split(',')
	print items
	if (flag == 'l'):
		# localhost:10001,localhost:10009,3.9675640
		host1=rldict[items[0]]
		host2=rldict[items[1]]
		
		oneTwo[(host1,host2)] = items[2].strip()
		twoOne[(host2,host1)] = items[2].strip()
		
		uniqueraw[items[0]]=""
		# twoOne[(host2,host1)] = items[2].strip()
		
		# cost=items[2].strip()
		# costdict[tuple(host1)]
		# new = host1 + "," + host2 + "," + cost
	elif (flag == 'v'):
		# localhost:10004,2
		new = rldict[items[0]]+ "," + items[1].strip()
		print new
		fout.write(new+"\n")
	else:
		print "flag not l or v"
		break

uniquedict = {}
if (flag == 'l'):
	for (host1,host2) in twoOne:
		if ((host2,host1) in twoOne):
			new = host1 + "," + host2 + "," + twoOne[(host2,host1)]
			uniquedict[host1]=""
			print new
			fout.write(new+"\n")
		else:
			print (host1 + "," + host2 + " is not bidirectional.")

print "there are "+str(len(uniqueraw))+" nodes pre-conversion."
print "there are "+str(len(uniquedict))+" nodes post-conversion."

print "test"+ str(len([1]))


			
fin.close()
fout.close()

print "all done."