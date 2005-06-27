#!/bin/sh

for i in `ps -ef | grep run_lookups | awk '{print $2}'` 
do
	kill -9 $i
done
for i in `ps -ef | grep run_node | awk '{print $2}'` 
do
	kill -9 $i
done
