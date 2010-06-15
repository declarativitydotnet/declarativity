set term postscript eps color dashed 18
#set term tgif dashed "Helvetia" 20
set output "hopcount.eps"
set size .6, .6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size .6, .6
set xlabel "Epoch Length (mins)" 
set ylabel "Hopcount" 
plot [5:40] [0:10]  "malicious.dat" using 1:10 title 'Maelstrom' w lp lt 1,\
          "" using 1:11 title 'Bamboo' w lp lt 2

set nomultiplot



