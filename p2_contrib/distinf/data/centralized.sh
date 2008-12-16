#!/bin/bash

if [ "$EXPROOT" == "" ]; then
    echo Please specify EXPROOT
    exit
fi

if [ $# -lt 3 ]; then
    echo "Usage: $0 <executable> <output-base> experiments..."
    echo experiment filenames must contain exactly one dot
    exit
fi

executable=$1
algdir=$EXPROOT/distinf/$2
shift 2

for i in $@; do
    mkdir -p $algdir/`dirname $i`
    base=`echo $i | cut -d'.' -f1`
    cmd="$PRLROOT/release/tests/$executable $base.net $algdir/$base"
    echo $cmd
    $cmd
done

