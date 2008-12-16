#!/bin/bash

workdir=~/workspace/distinf/p2
if [ $# -ne 1 ]; then
	echo 1>&2 'Usage: rl_runall <number of nodes>'
	exit 127
fi

# assign the addresses
for ((i=0;i<$1;i=i+1)) ; do
	nodenum[$i]=$((10+i))
done

for (( i=0; i<$1; i=i+1 )); do
	cmd="nohup $workdir/rl_local.sh $i >>& $workdir/logs/rl_local-r$((nodenum[$i])).log"
	echo $cmd 
	ssh r$((nodenum[$i])).millennium.berkeley.edu $cmd
done

#echo "sleep ...... 30s";
#sleep 30; # time for which the experiment should run
Pause()
{
   echo
   echo -n Hit Enter to continue....
   read
}

Pause

for (( i=0; i<$1; i=i+1 )); do 
	cmd="pkill runOverLog"
	echo $cmd 
	ssh r$((nodenum[$i])).millennium.berkeley.edu $cmd
done

