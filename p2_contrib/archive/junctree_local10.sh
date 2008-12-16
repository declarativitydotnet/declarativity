#!/bin/bash

# distInf-build/tests/runOverLog -p 10000 -o junctree.olg -DNODE_ADDR=\"localhost:10000\" -DMYID=\"0\" -DLINKS_FILE=\"data/links.csv\" -DVARS_FILE=\"data/variables2.csv\"

#overlog=distInf-build/tests/runOverLog
overlog=~/workspace/p2/trunk-build/tests/runOverLog

echo $overlog

N=10
#N=4

jtolg=junctree_static
links=data/local10-links-top20.csv
vars=data/local10_vars.csv
# links=data/local4_links.csv
# vars=data/local4_vars.csv

# First, assign the addresses
for ((i=0;i<N;i=i+1)) ; do
	port[$i]=$((10000+i))
	# xoffset[$i]=$((i*70))
	# yoffset[$i]=$((i*200))
	xoffset[$i]=$((i*20))
	yoffset[$i]=$((i*50))
done

for ((i=0;i<N;i=i+1)) ; do
	cp links.olg running/links.olg
	cp vars.olg running/vars.olg
	cp spantreerooted3.olg running/spantreerooted3.olg
	cp $jtolg.olg running/$i.$jtolg.olg
	a="$overlog -p $((port[$i])) -o running/$i.$jtolg.olg -DNODE_ADDR=\"localhost:$((port[$i]))\" -DMYID=\"$i\" -DLINKS_FILE=\"$links\" -DVARS_FILE=\"$vars\""
# >& logs/junctree-$((port[$i])).log"
	offset=i*10
	
	echo $a
	b="xterm -geometry 180x25+$((xoffset[$i]))+$((yoffset[$i])) -sb -sl 1000 -T $i -e $a"
	echo $b
	$b &
done
