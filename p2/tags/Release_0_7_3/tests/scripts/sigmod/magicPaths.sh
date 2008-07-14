#$1 - num nodes
#$2 - num queries
#$3 - query duration
#$4 - use cache
#$5 - percent dsts

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d grouchy/random/magic/random_$4_$5 -f random_$1_4_undirected -p 0.5 -m 0 -i doc/reachable_magic.plg -u -1 -c 0 -r 0 -D $3 -s 10 -q $2 -C $4 -P $5
