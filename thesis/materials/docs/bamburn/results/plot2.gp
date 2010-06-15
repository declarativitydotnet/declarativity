set term postscript eps
set output "graph2.eps"               

set origin 0,0 
set size 0.5, 0.5
set xlabel "Time (seconds)" 
set ylabel "Latency (ms)"
plot [0:1000] [0:300] "graph1.dat" using 1:3 w lp title 'No churn',"graph2.dat" using 1:3 w lp title 'With churn'
