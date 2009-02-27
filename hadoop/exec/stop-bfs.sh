#!/bin/sh
# Stop the BFS master node running on the local host.

bin=`dirname "$0"`
bin=`cd "$bin"; pwd`

. "$bin"/hadoop-config.sh

if [ -f "${HADOOP_CONF_DIR}/hadoop-env.sh" ]; then
  . "${HADOOP_CONF_DIR}/hadoop-env.sh"
fi

if [ "$HADOOP_PID_DIR" = "" ]; then
  HADOOP_PID_DIR=/tmp
fi

command=bfsmaster
pid=$HADOOP_PID_DIR/hadoop-$command.pid

if [ -f $pid ]; then
  if kill -0 `cat $pid` > /dev/null 2>&1; then
    echo stopping $command
    kill `cat $pid`
  else
    echo no $command to stop
  fi
else
  echo no $command to stop
fi

