#$1 - num nodes
#$2 - periodic
#$3 - 1 no share, 2 share

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d output/corr -f random_$1_4_undirected -p $2 -m 1 -i doc/reachable_correlate.plg -u -1 -c 0 -r $3 -D 60 -s -1 -q -1 -C 0 -P 0
