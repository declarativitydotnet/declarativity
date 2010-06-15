set term postscript eps color dashed 13
#set term tgif dashed "Helvetia" 20
set output "talk_results2.eps"
set size 1.0, 0.5
set data style linespoints

set multiplot

set origin 0.0,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" font "Helvetia, 18"
set ylabel "Routing table poisoning" font "Helvetia, 18"
set title "16 min churn" font "Helvetia, 18"
plot [0:3610] [0:1] "dats/16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
"dats/churn-point-16.dat" using 3:1 notitle  w lines lt 7,\
"dats/churn-point-16.dat" using 4:1 notitle w lines lt 7,\
"dats/churn-point-16.dat" using 5:1 notitle w lines lt 7
         
set origin 0.5,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" font "Helvetia, 18"
set ylabel "Routing table poisoning" 
set ylabel
set title "16 min churn: no rate-limiting" font "Helvetia, 18"
set key top right
plot [0:3610] [0:1] "dats/no-shield-16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1

set nomultiplot

