set term postscript eps color dashed 15 
set output "paths.eps"
set size 0.6, 0.6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size 0.6, 0.6
set grid 
set key 25,0.7
set ylabel "Prob. of Successful Routing"
set xlabel "% of Malicious node in Population" 
plot [4:30] [0:1] "paths.dat" using 1:4 title 'Bamboo' w lp lt 2,\
          "" using 1:2 title 'Maelstrom' w lp lt 1,\
          "" using 1:3 title 'Maelstrom (Redundancy = 16)' w lp lt 3
set nomultiplot 
