set term postscript eps color dashed 15 
set output "crt-time.eps"
set size 0.6, 0.6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size 0.6, 0.6
set grid 
set ylabel "CRT Poisoning (%)"
set xlabel "Time (secs)" 
set title "15% malicious fraction"
plot [0:3600] [0:1] "crt-15.dat" using 2:1 title 'Maintenance Redundancy = 0' w lp lt 3,\
          "crt-15-red-16-2.dat" using 1:2 title 'Maintenance Redundancy = 16' w lp lt 1,\
          0.25 notitle w lines lt 7
set nomultiplot 
