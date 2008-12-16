#!/bin/bash

echo Usage $0 algorithm niters [filenames]
alg=$1
niters=$2
app=random-ising
shift 2
../centralized.sh "loopy_bp -a $alg -e 0.2 -n $niters" $app/$alg $@
