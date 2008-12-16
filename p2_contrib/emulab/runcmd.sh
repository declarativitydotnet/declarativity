#!/bin/bash

if [ $# -lt 2 ]; then
        echo 1>&2 'Usage: runcmd <number of nodes> <local cmd>'
        exit 127
fi

N=$1
cluster=distinf$1
shift 1
remotecmd="$@"

dir=`pwd`

dir=${dir/\/home\/sfuniak\/selectsvn\/projects\/distinf\/p2/\/proj\/P2\/distinf\/distinf}
dir=${dir/\/Users\/ashima\/courses\/CS281\/Project\/uai\/P2/\/proj\/P2\/distinf\/distinf}
echo Remote directory: $dir

for (( i=1; i<=N; i=i+1 )); do 
	targethost=node-$i.$cluster.p2.emulab.net
	cmd="nohup $remotecmd  >& /tmp/cmd.log"
	echo $targethost: $cmd
	ssh -o StrictHostKeyChecking=no $targethost "cd $dir;$cmd" &
done

echo "Waiting for all processes to finish"
wait

#for (( i=1; i<=N; i=i+1 )); do 
#	targethost=node-$i.$cluster.p2.emulab.net
#        scp $targethost:/tmp/cmd.log $i.log
#done
