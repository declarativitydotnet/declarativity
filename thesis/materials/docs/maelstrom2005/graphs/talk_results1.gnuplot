set term postscript eps color dashed 13
#set term tgif dashed "Helvetia" 20
set output "talk_results1.eps"
set size 1.0, 1.0
set data style linespoints

set multiplot

set origin 0.0,0.5 
set size 0.5, 0.5
#set xlabel "Time (secs)" font "Helvetia, 18"
set xlabel
set ylabel "Routing Table Poisoning" font "Helvetia, 16"
set title "Maelstrom: 8 min churn" font "Helvetia, 18"
set key top right
plot [0:3610] [0:1] "dats/8min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
"dats/churn-point-8.dat" using 2:1 title 'Churn point' w lines lt 7,\
"dats/churn-point-8.dat" using 3:1 notitle  w lines lt 7,\
"dats/churn-point-8.dat" using 4:1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using 5:1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using 6:1  notitle w lines lt 7,\
"dats/churn-point-8.dat" using 7:1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using 8:1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using 8:1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using 9:1 notitle w lines lt 7
         
set origin 0.5,0.5 
set size 0.5, 0.5
#set xlabel "Time(secs)" font "Helvetia, 18"
set xlabel
#set ylabel "Routing table poisoning" font "Helvetia, 18"
set ylabel
set title "Maelstrom: 16 min churn" font "Helvetia, 18"
plot [0:3610] [0:1] "dats/16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
 "dats/churn-point-16.dat" using 2:1 title 'Churn point' w lines lt 7,\
"dats/churn-point-16.dat" using 3:1 notitle  w lines lt 7,\
"dats/churn-point-16.dat" using 4:1 notitle w lines lt 7,\
"dats/churn-point-16.dat" using 5:1 notitle w lines lt 7
         
set origin 0.0,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" font "Helvetia, 20"
set ylabel "Routing table poisoning" font "Helvetia, 16"
set title "Maelstrom: 32 min churn" font "Helvetia, 18"
plot [0:3610] [0:1] "dats/32min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
 "dats/churn-point-32.dat" using 2:1 title 'Churn point' w lines lt 7,\
"dats/churn-point-32.dat" using 3:1 notitle  w lines lt 7

 
set origin 0.5,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" font "Helvetia, 20"
#set ylabel "Routing table poisoning" font "Helvetia, 18"
set ylabel
set title "Bamboo" font "Helvetia, 18"
set key bottom right
plot [0:3610] [0:1] "dats/32min_bamboo_15mal_.dat" using 1:2 title 'Bamboo (15% Malicious)' w lp lt 1
 
set nomultiplot

