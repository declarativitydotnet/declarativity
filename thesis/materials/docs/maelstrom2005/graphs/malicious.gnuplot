set term postscript eps dashed 15
set linestyle lw 3
#set term tgif dashed "Helvetia" 20
set output "malicious.eps"
set size 1.6, 2.2
set data style linespoints

set multiplot

set origin 0.0,0.6 
set size 0.6, 0.6
set xlabel "Epoch Length (mins)" 
set ylabel "Routing Table Poisoning" 
set key 28,60 spacing 1.2 
plot [5:35] [0:100] "malicious.dat" using 1:2 title 'Maelstrom (5% Malicious)' w lp lt 1,\
          "" using 1:3 title 'Bamboo (5% Malicious)' w lp lt 2,\
          "" using 1:4 title 'Maelstrom (10% Malicious)' w lp lt 3,\
          "" using 1:5 title 'Bamboo (10% Malicious)' w lp lt 4
         
set origin 0.0,0.0 
set size 0.6, 0.6
set xlabel "Epoch Length (mins)" 
set ylabel "Routing Table Poisoning" 
set key 28,68
plot [5:35] [0:100]  "malicious.dat" using 1:6 title 'Maelstrom (15% Malicious)' w lp lt 1,\
          "" using 1:7 title 'Bamboo (15% Malicious)' w lp lt 2,\
          "" using 1:8 title 'Maelstrom (25% Malicious)' w lp lt 3,\
          "" using 1:9 title 'Bamboo (25% Malicious)' w lp lt 4
          
set nomultiplot

