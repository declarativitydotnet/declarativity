set term postscript eps color dashed 18
#set term tgif dashed "Helvetia" 20
set output "bw.eps"
set size .6, .6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size .6, .6
set xlabel "Epoch Length (mins)" 
set ylabel "Bandwidth (bytes/sec)" 
plot [5:35] [0:4500]  "bw.dat" using 1:2 title 'Maelstrom' w lp lt 1,\
          "" using 1:4 title 'Bamboo' w lp lt 2

set nomultiplot



