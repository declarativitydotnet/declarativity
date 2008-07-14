#$1 - num nodes
#$2 - periodic
#$3 - metric

./tests/scripts/sigmod/runLocalPaths.pl -n $1 -d grouchy/gtitm/nomagic/metric$3 -f gt-itm/ts$1-0.alt.out.$1 -p $2 -m $3 -i doc/reachable.plg -u -1 -c 0.1 -r 0 -D 60 -s -1 -q -1 -C 0 -P 0
