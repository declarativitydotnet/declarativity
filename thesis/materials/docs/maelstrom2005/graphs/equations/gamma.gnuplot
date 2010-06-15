set term postscript eps monochrome dashed 13
set output "gamma.eps"
set size .6, .4

set xlabel "Number of entries picked (s)" 
set ylabel "Expected lookups"
set boxwidth .5
set style fill solid .5
plot "gamma.dat" using 1:2 notitle w boxes


