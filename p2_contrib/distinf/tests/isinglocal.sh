#!/bin/bash
if [ $# -lt 2 ]; then
    echo "usage: $0 <algorithm> <num nodes> "
    exit 127
fi
./runlocal.sh $1 $2 random-ising 10x10_5 1
./runlocal.sh $1 $2 random-ising 10x10_5 2
./runlocal.sh $1 $2 random-ising 10x10_5 3
./runlocal.sh $1 $2 random-ising 10x10_5 4
./runlocal.sh $1 $2 random-ising 10x10_5 5
./runlocal.sh $1 $2 random-ising 10x10_5 6
./runlocal.sh $1 $2 random-ising 10x10_5 7
./runlocal.sh $1 $2 random-ising 10x10_5 8
./runlocal.sh $1 $2 random-ising 10x10_5 9
./runlocal.sh $1 $2 random-ising 10x10_5 10
./runlocal.sh $1 $2 random-ising 10x10_5 11
./runlocal.sh $1 $2 random-ising 10x10_5 12
./runlocal.sh $1 $2 random-ising 10x10_5 13
./runlocal.sh $1 $2 random-ising 10x10_5 14
./runlocal.sh $1 $2 random-ising 10x10_5 15
./runlocal.sh $1 $2 random-ising 10x10_5 16
./runlocal.sh $1 $2 random-ising 10x10_5 17
./runlocal.sh $1 $2 random-ising 10x10_5 18
./runlocal.sh $1 $2 random-ising 10x10_5 19
./runlocal.sh $1 $2 random-ising 10x10_5 20
./runlocal.sh $1 $2 random-ising 10x10_1 1
./runlocal.sh $1 $2 random-ising 10x10_1 2
./runlocal.sh $1 $2 random-ising 10x10_1 3
./runlocal.sh $1 $2 random-ising 10x10_1 4
./runlocal.sh $1 $2 random-ising 10x10_1 5
./runlocal.sh $1 $2 random-ising 10x10_1 6
./runlocal.sh $1 $2 random-ising 10x10_1 7
./runlocal.sh $1 $2 random-ising 10x10_1 8
./runlocal.sh $1 $2 random-ising 10x10_1 9
./runlocal.sh $1 $2 random-ising 10x10_1 10
