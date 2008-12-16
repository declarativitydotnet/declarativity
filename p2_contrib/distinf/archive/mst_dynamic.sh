#!/bin/bash

# ~/workspace/p2/trunk-build/tests/runOverLog -p 10001 -o spanningtree.olg -DNODE_ADDR=\"localhost:10001\" -DLINKS_FILE=\"links.csv\"

overlog=distInf-build/tests/runOverLog

echo $overlog

N=4

# First, assign the addresses
for ((i=0;i<N;i=i+1)) ; do
  port[$i]=$((10000+i))
done

for ((i=0;i<N;i=i+1)) ; do
	a="$overlog -p $((port[$i])) -o spantreedyn.olg -DNODE_ADDR=\"localhost:$((port[$i]))\" -DLINKS_FILE=\"links.csv\""
	# echo $a
	b="xterm -geometry 180x25 -T $i -e $a"
	echo $b
	$b >> logs/robustsp-$((port[$i])).log 2>&1 &
	# sleep 1
done
