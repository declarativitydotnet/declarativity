#!/bin/bash

echo Usage $0 exponent niters [filenames]
rho=$1
niters=$2
app=random-ising
alg=exponential
shift 2
../centralized.sh "loopy_bp -r $rho -a $alg -e 0.2 -n $niters" $app/$alg/$rho $@
