#$1 - num nodes
#$2 - update interval

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d grouchy/random -f random_$1_4_undirected -p 0.3 -m 1 -i doc/reachable.plg -u $2 -c 0.1 -r 0 -D 120 -s -1 -q -1 -C 0
