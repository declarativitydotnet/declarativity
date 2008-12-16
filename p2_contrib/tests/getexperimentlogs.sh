if [ $# -lt 6 ]; then
    echo 1>&2 "Usage: $0 <cluster> <algorithm> <nnodes> <epoch> <niters> [parameters] application <instances>"
    echo 1>&2 "$0 local rst 4 1 20 temperature-estimation model"
    echo 1>&2 "$0 local bp 4 1 100 BP_UPDATE_RATE=0.1 random-ising 10x10_5/12"
    echo 1>&2 "$0 distinf10 bp 9 4 20 image-segmentation 20x20"
    echo 1>&2 "$0 distinf16 jtinf 4 1 40 temperature-estimation model"
    echo 1>&2 "$0 local ap 5 3 160 AP_EPOCH=10 affinity-propagation 2D-25"
    exit
fi

target=`echo emulab`

cluster=$1
algoutput=$2
algorithm=$(echo $2 | awk -F \- '{print $1}')
N=$3
epoch=$4
niters=$5

shift 5

# Parse the optional arguments
while [ "`echo $1 | grep =`" ]; do
    parameters="$parameters $1"
    shift
done
app=$1
shift

echo $app
for instance in $@; do
    echo "Running on $app, instance $instance"
    basename=../data/$app/$instance
    tempdir=$cluster/$app/$algoutput/$target-$N/$instance
    echo $tempdir
    tempdir=temp/${tempdir//\//-}
    mkdir -p $tempdir
    rm -f $tempdir/*.log
    logfile=/p2/distinf/synchronize.log

    # Collect the results
    echo Copying the results
    for (( i=1; i<=N; i=i+1 )); do 
        targethost=node-$i.$cluster.p2.emulab.net
        logfile=/p2/distinf/run.log.gz
        echo "scp $targethost:$logfile $tempdir/$i.log.gz"
        nohup scp $targethost:$logfile $tempdir/$i.log.gz &
    done
    echo "nohup scp node-1.$cluster.p2.emulab.net:/p2/distinf/synchronize.log.gz $tempdir/synchronize.log.gz"
    scp node-1.$cluster.p2.emulab.net:/p2/distinf/synchronize.log.gz $tempdir/synchronize.log.gz &
    wait
    gunzip $tempdir/*.log.gz
    
    # Parse the log files
    resultbase=$EXPROOT/distinf/$app/$algoutput/$target-$N
    resultdir=$resultbase/`dirname $instance`
    echo $resultdir
    mkdir -p $resultdir

    pushd $tempdir
    echo "Algorithm : $algorithm"
    if [ "${algorithm:0:2}" = "ap" ]; then
        /opt/local/bin/python2.5 ../../extract_affinity.py $resultbase/$instance $N 6
    else
        ../../extract_beliefs.py $resultbase/$instance $N
        if [ "${algorithm:0:2}" != "bp" -o "$algorithm" == "bprand" ] && [ "${algorithm:0:2}" != "ap" ]; then
            ../../extract_jt.py $resultbase/$instance $N
        fi
    fi
    popd
        
    # Emulab doesn't let us overwrite other people's files, 
    # but we can erase them
    # rm -f $resultbase/$instance.txt
    #             echo $N $epoch $niters > $resultbase/$instance.txt

    echo .
done
