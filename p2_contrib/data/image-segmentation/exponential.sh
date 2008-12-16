#!/bin/bash

echo Usage $0 exponent niters [filenames]
rho=$1
niters=$2
app=image-segmentation
alg=exponential
shift 2
../centralized.sh "loopy_bp -r $rho -a $alg -b -n $niters" $app/$alg/$rho $@
