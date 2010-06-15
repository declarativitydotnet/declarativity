set term postscript eps color solid 12
set output "latencyCDF.eps"

set size .45, .5

set title ""
set xlabel "Latency (ms)"
set ylabel "CDF"
set key bottom right

plot [0:20000.] [0:1.1] "retry_latency.dat" title "Retry" with lines, \
      "reroute_latency.dat" title "Reroute" with lines
