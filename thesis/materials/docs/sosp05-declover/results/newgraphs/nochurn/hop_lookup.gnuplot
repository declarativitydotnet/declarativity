set term postscript eps
set output "hop_lookup.eps"

set size .5, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Hop Count"
set ylabel "Frequency"
set key top right

plot [-0.5:16] "simple_latency_hopdist-100.dat" using 1:3 title \
	     "100 nodes" with linespoints,\
	     "simple_latency_hopdist-300.dat" using 1:3 title \
	     "300 nodes" with linespoints,\
	     "simple_latency_hopdist-500.dat" using 1:3 title \
	     "500 nodes" with linespoints 
			
