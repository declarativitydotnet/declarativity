set term postscript eps color dashed 15 
set output "crt-paths.eps"
set size 0.6, 0.6
set data style linespoints

set multiplot


set origin 0.0,0.0 
set size 0.6, 0.6
set grid 
set key 25,70
set ylabel "Prob. of Successful Routing using CRT"
set xlabel "% Malicious Fraction in Population" 
plot [5:30] [0:109] "crt-paths.dat" using 1:2 title 'Lookup Redundancy = 0' w lp lt 3,\
          "" using 1:3 title 'Lookup Redundancy = 16' w lp lt 1
set nomultiplot 
