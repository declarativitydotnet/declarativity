#!/bin/bash

algo=$1
app=$2
dataset=$3
S=$4
N=$5

for ((i=S;i<=N;i=i+1)) ; do
	cmd=". $2local.sh $1 4 $3 $i"
	echo NOW RUNNING: $cmd
	$cmd
done
