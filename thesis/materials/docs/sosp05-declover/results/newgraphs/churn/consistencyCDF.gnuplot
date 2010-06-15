set term postscript eps
set output "consistentCDF-400-new.eps"

set size .45, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Consistent fraction"
set ylabel "CDF"
set key top left

set arrow 11 from graph 0.5, 1 to graph 0.5, 0 nohead lt 4
set label 12 "Consistency threshold" at graph 0.5, 0.5 center rotate

plot [0:1] "consistency-8.dat" using 3:4 title "8 min" with lines linewidth 2, \
     "consistency-16.dat" using 3:4 title "16 min" with lines linewidth 2, \
     "consistency-32.dat" using 3:4 title "32 min" with lines linewidth 2, \
    "consistency-64.dat" using 3:4 title "64 min" with lines linewidth 2, \
     "consistency-128.dat" using 3:4 title "128 min" with lines linewidth 2 \

