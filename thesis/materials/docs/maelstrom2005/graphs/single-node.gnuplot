set term postscript eps color dashed 15
#set term tgif dashed "Helvetia" 20
set output "single-node.eps"
set size 2.0, 0.5
set data style linespoints

set multiplot

set origin 0,0 
set size 0.5, 0.5
set xlabel "Time (secs)" 
set ylabel "Routing Table Poisoning" 
set title "Maelstrom - 8 min epoch length"
plot [0:3610] [0:1] "dats/8min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom (25% Malicious)' w lp lt 1
         
set origin 0.5,0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "Maelstrom - 16 min epoch length"
plot [0:3610] [0:1] "dats/16min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom (25% Malicious)' w lp lt 1
          
set origin 1.0,0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "Maelstrom - 32 min epoch length"
plot [0:3610] [0:1] "dats/32min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom (25% Malicious)' w lp lt 1
 
set origin 1.5,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "Bamboo"
plot [0:3610] [0:1] "dats/32min_bamboo_25mal_.dat" using 1:2 title 'Bamboo (25% Malicious)' w lp lt 1
 
set nomultiplot

