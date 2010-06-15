set term postscript eps color dashed 13
#set term tgif dashed "Helvetia" 20
set output "shield.eps"
set size 1.5, 2.0
set data style linespoints

set multiplot

        
set origin 0.0,1.5 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(a) Maelstrom: 16 min epoch length"
plot [0:3610] [0:1] "dats/16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
 0.24 title 'Average: steady state' w lines lt 3
         
set origin 0.5,1.5 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(b) Maelstrom: 32 min epoch length"
plot [0:3610] [0:1] "dats/32min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
 0.29 title 'Average: steady state' w lines lt 3

set origin 0.0,1.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(d) Maelstrom(no shield): 16 min epoch length"
plot [0:3610] [0:1] "no-shield/16min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom(no shield) (15% Malicious)' w lp lt 1,\
 0.41 title 'Average: steady state' w lines lt 3
         
set origin 0.5,1.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(e) Maelstrom(no shielding): 32 min epoch length"
plot [0:3610] [0:1] "no-shield/32min_maelstrom_15mal_.dat" using 1:2 title 'Maelstrom(no shield) (15% Malicious)' w lp lt 1,\
 0.47 title 'Average: steady state' w lines lt 3





set origin 1.0,1.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(f) Bamboo"
set key bottom right
plot [0:3610] [0:1] "dats/32min_bamboo_15mal_.dat" using 1:2 title 'Bamboo (15% Malicious)' w lp lt 1,\
 0.78 title 'Average: steady state' w lines lt 3
 
set origin 1.0,1.5
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(c) Bamboo (with shielding)"
set key bottom right
plot [0:3610] [0:1] "no-shield/32min_bamboo_15mal_.dat" using 1:2 title 'Bamboo+shield (15% Malicious)' w lp lt 1,\
 0.60 title 'Average: steady state' w lines lt 3
 



        
set origin 0.0,0.5 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(a) Maelstrom: 16 min epoch length"
plot [0:3610] [0:1] "dats/16min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom (25% Malicious)' w lp lt 1,\
 0.38 title 'Average: steady state' w lines lt 3
         
set origin 0.5,0.5 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(b) Maelstrom: 32 min epoch length"
plot [0:3610] [0:1] "dats/32min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom (25% Malicious)' w lp lt 1,\
 0.44 title 'Average: steady state' w lines lt 3

set origin 0.0,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(e) Maelstrom(no shield): 16 min epoch length"
plot [0:3610] [0:1] "no-shield/16min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom(no shield) (25% Malicious)' w lp lt 1,\
 0.52 title 'Average: steady state' w lines lt 3
         
set origin 0.5,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(f) Maelstrom(no shielding): 32 min epoch length"
plot [0:3610] [0:1] "no-shield/32min_maelstrom_25mal_.dat" using 1:2 title 'Maelstrom(no shield) (25% Malicious)' w lp lt 1,\
 0.57 title 'Average: steady state' w lines lt 3



set origin 1.0,0.0 
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(d) Bamboo"
set key bottom right
plot [0:3610] [0:1] "dats/32min_bamboo_25mal_.dat" using 1:2 title 'Bamboo (25% Malicious)' w lp lt 1,\
 0.85 title 'Average: steady state' w lines lt 3
 
set origin 1.0,0.5
set size 0.5, 0.5
set xlabel "Time(secs)" 
set ylabel "Routing table poisoning" 
set title "(d) Bamboo (with shielding)"
set key bottom right
plot [0:3610] [0:1] "no-shield/32min_bamboo_25mal_.dat" using 1:2 title 'Bamboo+shield (25% Malicious)' w lp lt 1,\
 0.73 title 'Average: steady state' w lines lt 3
 

set nomultiplot

