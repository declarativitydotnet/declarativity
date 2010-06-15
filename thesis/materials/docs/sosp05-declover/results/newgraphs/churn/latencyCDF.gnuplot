set term postscript eps
set output "churnLatencyCDF-400-new.eps"

set size .45, .5

set title ""
set nologscale xy
set tics out
set xlabel "Latency (s)"
set ylabel "CDF"
set key bottom right

plot [0:20] "simple_latency_cdf-8.dat" using 1:3 title "8 min" with lines linewidth 2, \
   "simple_latency_cdf-16.dat" using 1:3 title "16 min" with lines linewidth 2, \
   "simple_latency_cdf-32.dat" using 1:3 title "32 min" with lines linewidth 2, \
     "simple_latency_cdf-64.dat" using 1:3 title "64 min" with lines linewidth 2, \
     "simple_latency_cdf-128.dat" using 1:3 title "128 min" \
     with lines linewidth 2

			
