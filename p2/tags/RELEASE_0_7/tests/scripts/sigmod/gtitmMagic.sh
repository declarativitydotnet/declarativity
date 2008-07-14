#$1 - num nodes
#$2 - period between src/dst queries
#$3 - num queries

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d output/magic -f random_$1_4_undirected -p 0.5 -m 0 -i doc/reachable_magic.plg -u -1 -c 0 -r 0 -D 60 -s $2 -q $3
