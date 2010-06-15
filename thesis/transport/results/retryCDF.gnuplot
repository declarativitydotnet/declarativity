set term postscript eps color solid 12
set output "retryCDF.eps"

set size .45, .5

set title ""
set xlabel "Retry attempts"
set ylabel "CDF"
set key bottom right

plot [0:15.] [0:1.1] "retry_R.dat" title "Retry" with lines, \
      "reroute_R.dat" title "Reroute" with lines
