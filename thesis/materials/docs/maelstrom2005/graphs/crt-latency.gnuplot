set term postscript eps color dashed 15 
set output "crt-latency.eps"
set size 0.6, 0.6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size 0.6, 0.6
set grid 
set key 25,70
set ylabel "Network Latency (msecs)"
set xlabel "Epoch Length (mins)" 
plot [5:35] [0:680] "crt-latency.dat" using 1:2 title 'Constrained RT' w lp lt 3,\
          "" using 1:3 title 'Bamboo proximity RT' w lp lt 1
set nomultiplot 
