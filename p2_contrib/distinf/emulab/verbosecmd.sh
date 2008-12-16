#!/bin/bash

source `dirname $0`/runcmd.sh

for (( i=1; i<=N; i=i+1 )); do 
	targethost=node-$i.$cluster.p2.emulab.net
        scp $targethost:/tmp/cmd.log $i.log
done
