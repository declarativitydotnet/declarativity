set terminal postscript eps
set output "graph5.eps"

set origin 0,0 
set size 0.5, 0.5
set xlabel "Size of unspoofable identifier (bits)"
set ylabel "% of Consistent Lookups"
plot [24:30] [0:110.0] "graph5.dat" using 1:2 w lp title 'Without diversity',"graph5.dat" using 1:3 w lp title 'With diversity'



