set terminal postscript eps
set output "graph1.eps"

set origin 0,0 
set size 0.5, 0.5
set xlabel "Time (seconds)"
set ylabel "% of Consistent Lookups"
plot [0:1000.0] [0:120.0] "graph1.dat" using 1:2 w lp title 'No churn',"graph2.dat" using 1:2 w lp title 'With churn'



