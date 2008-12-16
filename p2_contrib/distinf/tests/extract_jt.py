#! /usr/bin/env python

# run the script:
# ./printgraph.py kuang/test10.out kuang/test10.dot data/local10-links-top20.csv
# ./printgraph.py kuang/test4.out kuang/test4.dot local4_links.csv

import sys, re, os
import time, datetime
#print sys.argv

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
#     d = time.strptime(items[0],'%Y-%b-%d %H:%M:%S')
#     d0 = datetime.datetime(2000, 1, 1, 0, 0, 0)
#     diff = datetime.datetime(d[0],d[1],d[2],d[3],d[4],d[5])-d0
#     return 730486+diff.days+(diff.seconds+float("0."+items[1]))/86400.0

if len(sys.argv)!=3:
    print 'Usage: save_jt.py <filename base> <number of nodes>' 
    sys.exit(1)

base = sys.argv[1]
count = int(sys.argv[2])

id = {}

print "Count = " + str(count)

print "Extracting the ids..."
for i in range(1, count+1):
    #print "Parse log for node " + str(i)
    filein = str(i) + ".log"

    for line in open(filein):
        if re.search('AddAction',line) and re.search('identifier\(', line):
            items = parse_line(line, 'identifier\(')
            #print items
            id[items[0]] = items[1]

fedges = open(base+"-edges.txt", "w")
fparents = open(base+"-parents.txt", "w")
fcliques = open(base+"-cliques.txt","w")
    
print "Extracting the edges..."
for i in range(1, count+1):
    #print "Parsing log for node " + str(i)
    filein = str(i) + ".log"
    
    for line in open(filein):
        if re.search('SendAction', line) and re.search('edge_inserted\(', line):
            items = parse_line(line, 'edge_inserted\(')
            fedges.write(id[items[0]]+' '+id[items[1]]+' 1 '+items[2]+'\n')

        if re.search('SendAction', line) and re.search('edge_deleted\(', line):
            items = parse_line(line, 'edge_deleted\(')
            fedges.write(id[items[0]]+' '+id[items[1]]+' 0 '+items[2]+'\n')

        if re.search('SendAction', line) and re.search('parent_changed\(', line):
            items = parse_line(line, 'parent_changed\(')
            fparents.write(id[items[0]]+' '+id[items[1]]+' '+items[2]+'\n')
        if re.search('SendAction', line) and re.search('clique_inserted\(', line):
            items = parse_line(line, 'clique_inserted\(')
            fcliques.write(id[items[0]]+' '+items[1]+' 1 '+items[2]+'\n')
        if re.search('SendAction', line) and re.search('clique_deleted\(', line):
            items = parse_line(line, 'clique_deleted\(')
            fcliques.write(id[items[0]]+' '+items[1]+' 0 '+items[2]+'\n')


fedges.close()
fparents.close()
fcliques.close()
