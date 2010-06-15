set term postscript eps color solid 12
set output "hopCDF.eps"

set size .45, .5

set title ""
set xlabel "Hop Count"
set ylabel "CDF"
set key bottom right

plot [0:5.] [0:1.1] "retry_hop.dat" title "Retry" with lines, \
      "reroute_hop.dat" title "Reroute" with lines
