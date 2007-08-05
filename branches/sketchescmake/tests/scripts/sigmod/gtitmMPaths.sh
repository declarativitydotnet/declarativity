#$1 - num nodes
#$2 - cache or not
#$3 - number of queries
#$4 - query duration
#$5 - Percent of destinations

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d grouchy/gtitm/magic/hubmetric0_$2_$5 -f gt-itm/ts$1-0.alt.out.$1 -p 0.5 -m 0 -i doc/reachable_magic.plg -u -1 -c 0.1 -r 0 -D $4 -s 10 -q $3 -C $2 -P $5
