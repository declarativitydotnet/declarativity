#!/bin/bash

overlog=distInf-build/tests/runOverLog
echo $overlog

N=$1
link_file=$2

for ((i=1;i<=N;i=i+1)) ; do
  port=$((10000+i))
  xterm -geometry 150x25 -T port$port -e $overlog -o spanningtree.olg -DNODE_ADDR=\"localhost:$port\" -DLINK_FILE=\"$link_file\" -p $port -n localhost  &
  sleep 1
done

