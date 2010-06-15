set term postscript eps color dashed 13
#set term tgif dashed "Helvetia" 20
set output "combined2.eps"
set size 1.5, 1.0
set data style linespoints

set multiplot

set origin 0.0,0.5 
set size 0.5, 0.5
set xlabel "Time (secs)" 
set ylabel "Routing Table Poisoning" 
set title "(a) Maelstrom: 8 min epoch length"
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
set xlabel "Time(secs)" 
#set ylabel "Routing table poisoning" 
set ylabel
set title "(b) Maelstrom: 16 min epoch length"
plot [0:3610] [0:1] "dats/16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
 "dats/churn-point-16.dat" using 2:1 title 'Churn point' w lines lt 7,\
"dats/churn-point-16.dat" using 3:1 notitle  w lines lt 7,\
"dats/churn-point-16.dat" using 4:1 notitle w lines lt 7,\
"dats/churn-point-16.dat" using 5:1 notitle w lines lt 7,\
"dats/churn-point-16.dat" using 6:1 notitle w lines lt 7
         
set origin 1.0,0.5 
set size 0.5, 0.5
#set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set ylabel
set title "(c) Maelstrom: 32 min epoch length"
plot [0:3610] [0:1] "dats/32min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
 "dats/churn-point-32.dat" using 2:1 title 'Churn point' w lines lt 7,\
"dats/churn-point-32.dat" using 3:1 notitle  w lines lt 7

 
set origin 0.0,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(d) Bamboo"
set key bottom right
plot [0:3610] [0:1] "dats/32min_bamboo_15mal_.dat" using 1:2 title 'Bamboo (15% Malicious)' w lp lt 1
 
set origin 0.5,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
#set ylabel "Routing table poisoning" 
set ylabel
set title "(e) Maelstrom without shielding: 16 min epoch length"
set key top right
plot [0:3610] [0:1] "dats/no-shield-16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1
 


set origin 1.0,0.0 
set size 0.5, 0.5
set xlabel "Malicious fraction in population" 
#set ylabel "Routing Table Poisoning" 
set ylabel
set key 31,0.74 
set title "(f) RT Poisoning"
set xtics ("5" 5, "10" 10, "15" 15, "20" 20, "25" 25)
plot [0:38] [0:1] "combined2.dat" using 1:2 title 'Maelstrom (8min)' w lp lt 1,\
          "" using 1:3 title '(16min)' w lp lt 2,\
          "" using 1:4 title '(32min)' w lp lt 3,\
          "" using 1:5 title 'Bamboo' w lp lt 4



set nomultiplot

