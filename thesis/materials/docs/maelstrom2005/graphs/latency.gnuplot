set term postscript eps color dashed 18 
set output "latency.eps"
set size .6, .6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size .6, .6
set grid 
set ylabel "Latency(msecs)"
set xlabel "Epoch Length(mins)" 
plot [5:35] [0:510] "latency.dat" using 1:3 title 'Bamboo' w lp lt 3,\
          "" using 1:2 title 'Maelstrom' w lp lt 1
set nomultiplot 
