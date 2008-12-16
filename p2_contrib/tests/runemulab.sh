# Invoked from run.sh
# Fix the output DB location

# We may have to move all the sources to local file system as well
OVERLOG=/p2/distinf/release/bin/runStagedOverlog
port=22222
echo Executing $OVERLOG

# Create a list of node ids and addresses
nodes=$tempdir/nodes.csv
rm -f $nodes
for ((i=1;i<=N;i=i+1)); do
    echo "$i,node-$i.$cluster.p2.emulab.net:$port" >> $nodes
done
echo "Done,Done" >> $nodes
 
# Compress the relevant files
cd ..
rm -f experiment.tar.gz
inputs="$links $vars $edges $nodepot $edgepot $cliquepot $similarity $preference"
echo "tar cvfz experiment.tar.gz overlog/*.olg tests/*.{olg,csv,sh} tests/$nodes ${inputs//..\//} > /dev/null"
tar cvfz experiment.tar.gz overlog/*.olg tests/*.{olg,csv,sh} tests/$nodes ${inputs//..\//} > /dev/null
cd tests

# Start the experiment
echo Starting the synchronization program
targethost=node-1.$cluster.p2.emulab.net
logfile=/p2/distinf/synchronize.log
scp ../experiment.tar.gz emulab_local.sh $targethost:/tmp
cmd="/tmp/emulab_local.sh $OVERLOG $logfile -p 9999 -n node-1.$cluster.p2.emulab.net -r WARN -DNODES_FILE=\\\"$nodes\\\" -o ../overlog/synchronize.olg"
echo $cmd
nohup ssh -o StrictHostKeyChecking=no $targethost $cmd &
sleep 5 # let is start

echo Starting $N instances
for (( i=1; i<=N; i=i+1 )); do
    targethost=node-$i.$cluster.p2.emulab.net
    cmd="/tmp/emulab_local.sh $OVERLOG /p2/distinf/run.log \
                $lossy_links \
                $parameters \
                -z \
                -r WARN \
                -n $targethost \
                -p $port \
                -DBASE_ADDR=\\\"node-1.$cluster.p2.emulab.net:9999\\\" \
                -DLANDMARK=\\\"node-1.$cluster.p2.emulab.net:$port\\\" \
                -DNNODES=$N \
                -DMYID=$i \
                -DNODES_FILE=\\\"$nodes\\\" \
                -DLINKS_FILE=\\\"$links\\\" \
                -DVARS_FILE=\\\"$vars\\\" \
                -DMRF_EDGES_FILE=\\\"$edges\\\" \
                -DNODE_POT_FILE=\\\"$nodepot\\\" \
                -DEDGE_POT_FILE=\\\"$edgepot\\\" \
                -DINPUT_DB=\\\"$input_db\\\" \
                -DOUTPUT_DIR=\\\"$tempdir\\\" \
                -DSIMILARITY_FILE=\\\"$similarity\\\" \
                -DPREFERENCE_FILE=\\\"$preference\\\" \
                -o $algorithm.olg"
    echo $cmd 
    if [ $i == 1 ]; then
        rm -f $tempdir/compile.log
        echo "(compiled at each node separately)" > $tempdir/compile.log
        echo $cmd >> $tempdir/compile.log
    fi
    (scp ../experiment.tar.gz emulab_local.sh $targethost:/tmp; nohup ssh -o StrictHostKeyChecking=no $targethost $cmd) &
done
# Also consider CheckHostIp=no 

echo "Waiting for all instances to finish"
wait

# Collect the results
echo Copying the results
for (( i=1; i<=N; i=i+1 )); do 
    targethost=node-$i.$cluster.p2.emulab.net
    logfile=/p2/distinf/run.log.gz
    echo "nohup scp $targethost:$logfile $tempdir/$i.log.gz"
    nohup scp $targethost:$logfile $tempdir/$i.log.gz &
done
echo "scp node-1.$cluster.p2.emulab.net:/p2/distinf/synchronize.log.gz $tempdir/synchronize.log.gz"
nohup scp node-1.$cluster.p2.emulab.net:/p2/distinf/synchronize.log.gz $tempdir/synchronize.log.gz &
wait

gunzip $tempdir/*.log.gz
