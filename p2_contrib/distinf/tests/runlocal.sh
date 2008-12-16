# Invoked from run.sh

#OVERLOG="valgrind --tool=callgrind $OVERLOG"
#echo $OVERLOG

# Create a list of node ids and addresses
nodes=$tempdir/nodes.csv
rm -f $nodes
for ((i=1;i<=N;i=i+1)); do
    port=$((10000+i))
    echo "$i,localhost:$port" >> $nodes
done
echo "Done,Done" >> $nodes

# Erase old database files
rm -f $tempdir/*.db

# Compile the source file
echo Compiling the input file
rm -f $algorithm.olg.df # VERY important
cmd="$OVERLOG -C \
                        $lossy_links \
                        $parameters \
                        -r WARN \
                        -DBASE_ADDR=\"localhost:9999\" \
                        -DLANDMARK=\"localhost:10001\" \
                        -DNNODES=$N \
                        -DMYID=\"MYID\" \
                        -DNODES_FILE=\"$nodes\"
                        -DLINKS_FILE=\"$links\" \
                        -DVARS_FILE=\"$vars\" \
                        -DMRF_EDGES_FILE=\"$edges\" \
                        -DNODE_POT_FILE=\"$nodepot\" \
                        -DEDGE_POT_FILE=\"$edgepot\" \
                        -DINPUT_DB=\"$input_db\" \
                        -DOUTPUT_DIR=\"$tempdir\" \
                        -DSIMILARITY_FILE=\"$similarity\" \
                        -DPREFERENCE_FILE=\"$preference\" \
                        -o $algorithm.olg"

echo $cmd
$cmd 2>&1 | tee $tempdir/compile.log 
echo $cmd >> $tempdir/compile.log

# Start the experiment
echo Starting the synchronization program
cmd="$OVERLOG -p 9999 -DNODES_FILE=\"$nodes\" -o ../overlog/synchronize.olg"
echo $cmd
$cmd 2>$tempdir/synchronize.log &
sleep 3

echo Starting $N instances
for ((i=1;i<=N;i=i+1)) ; do
    port=$((10000+i))
    cp -p $algorithm.olg .$i.olg
    cat $algorithm.olg.df | sed "s/\\\\\"MYID\\\\\"/$i/g" | sed "s/localhost:10000/localhost:$port/g" > .$i.olg.df

    cmd="$OVERLOG $measure_bandwidth -r WARN -p $port -DNNODES=$N -o .$i.olg"
    echo $cmd
    $cmd 2>$tempdir/$i.log &
done

echo "Waiting for all instances to finish"
wait
# echo "Press Enter to kill P2"
# read
# ./killp2

