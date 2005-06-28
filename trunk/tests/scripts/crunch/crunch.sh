# $1 is number of nodes
# $2 is the location of the logs
rm $2/*.dat
../pout.py -e -n $1 $2

./buildCDF.pl -f simple_latency.dat > $2/simple_latency_cdf.dat
./hopDist.pl -f simple_latency.dat > $2/simple_latency_hopdist.dat
mv simple_latency.dat $2/.
mv maintenance_latency.dat $2/.
mv node_bw.dat $2/.
mv simple_hop_time.dat $2/.
mv maintenance_hop_time.dat $2/.

