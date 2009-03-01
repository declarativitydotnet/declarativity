#!/bin/sh
# Start a BFS data node on all the addresses in the "slaves" file.

bin=`dirname "$0"`
bin=`cd "$bin"; pwd`

. "$bin"/hadoop-config.sh

if [ -f "${HADOOP_CONF_DIR}/hadoop-env.sh" ]; then
  . "${HADOOP_CONF_DIR}/hadoop-env.sh"
fi

"$bin"/hadoop-daemons.sh --config $HADOOP_CONF_DIR start bfsdatanode

