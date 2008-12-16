#!/bin/bash

overlog=~/workspace/distinf/runOverLog_new

echo $overlog

N=4
jtolg=local4static

# First, assign the addresses
for ((i=1;i<=N;i=i+1)) ; do
	port[$i]=$((10000+i))
	# xoffset[$i]=$((i*70))
	# yoffset[$i]=$((i*200))
	xoffset[$i]=$((i*20))
	yoffset[$i]=$((i*50))
done

for ((i=1;i<=N;i=i+1)) ; do
	cp shafer_shenoy.olg running/shafer_shenoy.olg
	cp intel-data/intel4-model.olg running/intel4-model.olg
	cp $jtolg.olg running/$i.$jtolg.olg
	a="$overlog -p $((port[$i])) -o running/$i.$jtolg.olg -DMYID=$i"
	echo $a
	b="xterm -geometry 180x25+$((xoffset[$i]))+$((yoffset[$i])) -sb -sl 1000 -T $i -e $a"
	echo $b
	$b &
done
