set term postscript eps color dashed 15 
set output "crt.eps"
set size 0.6, 0.6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size 0.6, 0.6
set grid 
set ylabel "CRT Poisoning (%)"
set xlabel "% Malicious Fraction in Population" 
plot [0:30] [0:50] "crt.dat" using 1:2 title 'Maintenance Redundancy = 0' w lp lt 3,\
          "" using 1:3 title 'Maintenance Redundancy = 16' w lp lt 1
set nomultiplot 
