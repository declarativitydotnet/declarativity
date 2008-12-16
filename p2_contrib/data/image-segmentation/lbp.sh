#!/bin/bash

echo Usage $0 algorithm niters [filenames]
alg=$1
niters=$2
app=image-segmentation
shift 2
../centralized.sh "loopy_bp -a $alg -b -n $niters" $app/$alg $@
