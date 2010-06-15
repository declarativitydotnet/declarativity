set term postscript eps
set output "noChurnLatencyCDF.eps"

set size .45, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Latency (s)"
set ylabel "CDF"
set key bottom right

plot [0:10] "simple_latency_cdf-100.dat" using 1:3 title "100 nodes" \
     with lines linewidth 2, \
     "simple_latency_cdf-300.dat" using 1:3 title "300 nodes" \
     with lines linewidth 2, \
     "simple_latency_cdf-500.dat" using 1:3 title "500 nodes" \
     with lines linewidth 2
			
