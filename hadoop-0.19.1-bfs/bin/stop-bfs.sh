#!/bin/sh
# Stop the BFS master node running on all the addresses in the "masters" file.

bin=`dirname "$0"`
bin=`cd "$bin"; pwd`

. "$bin"/hadoop-config.sh

if [ -f "${HADOOP_CONF_DIR}/hadoop-env.sh" ]; then
  . "${HADOOP_CONF_DIR}/hadoop-env.sh"
fi

"$bin"/hadoop-masters.sh --config $HADOOP_CONF_DIR stop bfsmaster

