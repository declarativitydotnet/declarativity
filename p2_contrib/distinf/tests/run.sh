#!/bin/bash

if [ "$EXPROOT" == "" ]; then
    echo Please set EXPROOT to the directory where the results will be stored
    echo "(must be an absolute path)"
    exit
fi

if [ $# -lt 6 ]; then
    echo 1>&2 "Usage: $0 <cluster> <algorithm> <nnodes> <epoch> <niters> [parameters] application <instances>"
    echo 1>&2 "$0 local rst 4 1 20 temperature-estimation model"
    echo 1>&2 "$0 local bp 4 1 100 BP_UPDATE_RATE=0.2 random-ising 10x10_5/12"
    echo 1>&2 "$0 distinf10 bp 9 4 20 image-segmentation 20x20"
    echo 1>&2 "$0 distinf16 jtinf 4 1 40 temperature-estimation model"
    echo 1>&2 "$0 local ap 5 6 100 AP_EPOCH=6 affinity-propagation 2D-25 5"
    exit
fi

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

echo "Running $algorithm with $N nodes, epoch=$epoch, niters=$niters"
echo "Optional arguments: $parameters"

parameters="$parameters EPOCH=$epoch NITERS=$niters"
parameters=${parameters//\ / -D}

measure_bandwidth="-z"    #comment out to disable bandwidth logging
lossy_links="-DLOSSY_LINKS=1" 

# The remaining entries are the input instances
app=$1
shift

echo $app

edges=
nodepot=
edgepot=
cliquepot=

if [ "$cluster" == "local" ]; then
    target=localhost
else
    target=emulab
fi

for instance in $@; do
    
    echo "Running on $app, instance $instance"

    basename=../data/$app/$instance

    # Compute the input files
    if [ "${algorithm:0:3}" = "rst" ] || [ "${algorithm:0:2}" = "jt" ]; then
        links=$basename-links-$N.csv
        input_db=$basename-$N.db
    elif [ "${algorithm:0:2}" = "bp" ]; then
        vars=$basename-vars-$N.csv
        links=$basename-links-$N.csv
        edges=$basename-edges.csv
        nodepot=$basename-nodepot.csv
        edgepot=$basename-edgepot.csv
    elif [ "${algorithm:0:2}" = "ap" ]; then
        vars=$basename-vars-$N.csv
        similarity=$basename-similarity-$N.csv
        preference=$basename-preference-$N.csv
    else
        echo "Unknown algorithm: $algorithm"
        exit -1
    fi

    # Create the log directory
    tempdir=$cluster/$app/$algoutput/$target-$N/$instance
    tempdir=temp/${tempdir//\//-}
    mkdir -p $tempdir
    rm -f $tempdir/*.log
    echo "Temporary directory: $tempdir"

    # Execute the experiment
    if [ "$cluster" == "local" ]; then
        source runlocal.sh
    else
        source runemulab.sh
    fi

    # Parse the log files
    resultbase=$EXPROOT/distinf/$app/$algoutput/$target-$N
    resultdir=$resultbase/`dirname $instance`
    echo $resultdir
    mkdir -p $resultdir

    pushd $tempdir
    echo "Algorithm : $algorithm"
    if [ "${algorithm:0:2}" = "ap" ]; then
        # hard coded the number of variables per node. Need to update this
        ../../extract_affinity.py $resultbase/$instance $N 10
    else        
        ../../extract_beliefs.py $resultbase/$instance $N
        if [ "${algorithm:0:2}" != "bp" -o "$algorithm" == "bprand" ] && [ "${algorithm:0:2}" != "ap" ]; then
            ../../extract_jt.py $resultbase/$instance $N
        fi
        # Collect the results into a single database
        echo "Merging the database $resultbase/$instance.db"
        rm -f $resultbase/$instance.db
        for ((i=1;i<=N;i=i+1)); do
            sqlite3 -batch $i.db .dump | sqlite3 -batch $resultbase/$instance.db
        done
    fi
    popd

    # Emulab doesn't let us overwrite other people's files, 
    # but we can erase them
    rm -f $resultbase/$instance.txt
    echo $N $epoch $niters > $resultbase/$instance.txt

    echo .
done


