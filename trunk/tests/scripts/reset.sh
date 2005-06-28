#!/bin/sh

for i in `ps -ef | grep run_lookups | awk '{print $2}'` 
do
	kill -9 $i >> $1/reset.log 2>&1
done
for i in `ps -ef | grep run_node | awk '{print $2}'` 
do
	kill -9 $i >> $1/reset.log 2>&1
done
echo "\n\n============= REMAINING PROCESSES =============" >> $1/reset.log 2>&1
ps -ef | grep `whoami`>> $1/reset.log 2>&1
echo "===============================================\n\n" >> $1/reset.log 2>&1
