#!/bin/bash

# /home/eecs/kuangc/workspace/p2/trunk-build/tests/runOverLog -p 22222 -n r11.millennium.berkeley.edu -o junctree_static.olg -DNODE_ADDR=\"r11.millennium.berkeley.edu:22222\" -DMYID=\"1\" -DLINKS_FILE=\"data/rl_links.csv\" -DVARS_FILE=\"data/rl_vars.csv\"

#overlog=distInf-build/tests/runOverLog
overlog=/home/eecs/kuangc/workspace/p2/trunk-build/tests/runOverLog
port=22222
host=$HOSTNAME.millennium.berkeley.edu

id=$1
echo id $id

workdir=~/workspace/distinf/p2
cd $workdir
if [ -e $workdir/logs/jt-$HOSTNAME.log ];
then 
    mv $workdir/logs/jt-$HOSTNAME.log $workdir/logs/jt-$HOSTNAME.old
fi

if [ -e $workdir/logs/rl_local-$HOSTNAME.log ];
then 
    mv $workdir/logs/rl_local-$HOSTNAME.log $workdir/logs/rl_local-$HOSTNAME.old
fi
cmd="$overlog -p $port -n $host -o junctree_static.olg -DNODE_ADDR=\"$host:$port\" -DMYID=\"$id\" -DLINKS_FILE=\"data/rl10_links_top20.csv\" -DVARS_FILE=\"data/rl10_vars.csv\""
echo $cmd
$cmd > $workdir/logs/jt-$HOSTNAME.log 2>&1 &
#$cmd 2>&1 1> $workdir/logs/jt-$HOSTNAME.log &

