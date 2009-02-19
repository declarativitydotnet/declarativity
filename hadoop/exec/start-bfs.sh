#!/bin/sh
# Start a copy of the BFS master node. Right now, we run the BFS
# master in the foreground.

bin=`dirname "$0"`
bin=`cd "$bin"; pwd`

. "$bin"/hadoop-config.sh

# The $CLASSPATH and $JAVA stuff copied from bin/hadoop, which is a
# bit hacky
JAVA=$JAVA_HOME/bin/java

# CLASSPATH initially contains $HADOOP_CONF_DIR
CLASSPATH="${HADOOP_CONF_DIR}"
CLASSPATH=${CLASSPATH}:$JAVA_HOME/lib/tools.jar

# for developers, add Hadoop classes to CLASSPATH
if [ -d "$HADOOP_HOME/build/classes" ]; then
  CLASSPATH=${CLASSPATH}:$HADOOP_HOME/build/classes
fi
if [ -d "$HADOOP_HOME/build/webapps" ]; then
  CLASSPATH=${CLASSPATH}:$HADOOP_HOME/build
fi
if [ -d "$HADOOP_HOME/build/test/classes" ]; then
  CLASSPATH=${CLASSPATH}:$HADOOP_HOME/build/test/classes
fi

# so that filenames w/ spaces are handled correctly in loops below
IFS=

# for releases, add core hadoop jar & webapps to CLASSPATH
if [ -d "$HADOOP_HOME/webapps" ]; then
  CLASSPATH=${CLASSPATH}:$HADOOP_HOME
fi
for f in $HADOOP_HOME/hadoop-*-core.jar; do
  CLASSPATH=${CLASSPATH}:$f;
done

# add libs to CLASSPATH
for f in $HADOOP_HOME/lib/*.jar; do
  CLASSPATH=${CLASSPATH}:$f;
done

for f in $HADOOP_HOME/lib/jetty-ext/*.jar; do
  CLASSPATH=${CLASSPATH}:$f;
done

for f in $HADOOP_HOME/hadoop-*-tools.jar; do
  TOOL_PATH=${TOOL_PATH}:$f;
done
for f in $HADOOP_HOME/build/hadoop-*-tools.jar; do
  TOOL_PATH=${TOOL_PATH}:$f;
done

# add user-specified CLASSPATH last
if [ "$HADOOP_CLASSPATH" != "" ]; then
  CLASSPATH=${CLASSPATH}:${HADOOP_CLASSPATH}
fi

export HADOOP_IDENT_STRING=""
command="bfsmaster"
export HADOOP_LOGFILE=hadoop-$HADOOP_IDENT_STRING-$command-$HOSTNAME.log
log=$HADOOP_LOG_DIR/hadoop-$HADOOP_IDENT_STRING-$command-$HOSTNAME.out
pid=$HADOOP_PID_DIR/hadoop-$HADOOP_IDENT_STRING-$command.pid

$JAVA -cp "$CLASSPATH" bfs.Master 0 > "$log" 2>&1 < /dev/null &
echo $! > $pid

