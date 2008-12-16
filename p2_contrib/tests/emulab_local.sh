#!/bin/bash

cd /p2/distinf
tar zxf /tmp/experiment.tar.gz
cd tests
cmd=$1
logfile=$2
shift 2
rm -f $logfile.gz
$cmd $@ >& $logfile
gzip $logfile
