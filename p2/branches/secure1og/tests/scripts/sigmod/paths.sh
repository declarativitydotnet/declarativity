#$1 - num nodes
#$2 - periodic

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d grouchy/random/nomagic/random -f random_$1_4_undirected -p $2 -m 0 -i doc/reachable.plg -u -1 -c 0 -r 0 -D 30 -s -1 -q -1 -C 0 -P 0
