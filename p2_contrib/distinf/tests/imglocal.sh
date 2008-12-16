#!/bin/bash
if [ $# -lt 4 ]; then
    echo "usage: $0 <algorithm> <num nodes> <data> <datasize>"
    exit 127
fi
./runlocal.sh $1 $2 image-segmentation $3 $4
