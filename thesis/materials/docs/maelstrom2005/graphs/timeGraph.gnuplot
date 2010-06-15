set term postscript eps color dashed 13
#set term tgif dashed "Helvetia" 20
set output "timeGraph.eps"
set size 1.4, .4
set data style linespoints
set pointsize 0.5

set multiplot

set origin 0.0, 0.0 
set size 0.38, 0.4
set xlabel "Time (min)" 
set ylabel "Routing Table Poisoning" 
#set key top right
set nokey
set label 11 "(a) 8 min epoch" at graph .5,.8 center
plot [-1:65] [0:1] "dats/8min_maelstrom_15mal_.dat" using ($1/60):2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
"dats/churn-point-8.dat" using ($2/60):1 title 'Churn point' w lines lt 7,\
"dats/churn-point-8.dat" using ($3/60):1 notitle  w lines lt 7,\
"dats/churn-point-8.dat" using ($4/60):1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using ($5/60):1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using ($6/60):1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using ($7/60):1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using ($8/60):1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using ($8/60):1 notitle w lines lt 7,\
"dats/churn-point-8.dat" using ($9/60):1 notitle w lines lt 7


set origin 0.32, 0.0
set size 0.32, 0.4
set xlabel "Time (min)" 
set ylabel "" 
set format y ""
set label 11 "(b) 16 min epoch" at graph .5,.8 center
plot [-1:65] [0:1] "dats/16min_maelstrom_15mal_.dat" using ($1/60):2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
"dats/churn-point-16.dat" using ($2/60):1 title 'Churn point' w lines lt 7,\
"dats/churn-point-16.dat" using ($3/60):1 notitle w lines lt 7,\
"dats/churn-point-16.dat" using ($4/60):1 notitle w lines lt 7,\
"dats/churn-point-16.dat" using ($5/60):1 notitle w lines lt 7


set origin 0.58, 0.0 
set size 0.32, 0.4
set xlabel "Time (min)" 
#set ylabel "Routing table poisoning" 
set ylabel
set label 11 "(c) 16 min epoch\nno shielding" at graph .5,.8 center
#set key top right
set nokey
plot [-1:65] [0:1] "dats/no-shield-16min_maelstrom_15mal_.dat" using ($1/60):2 title 'Maelstrom (15% Malicious)' w lp lt 1


set origin 0.84, 0.0 
set size 0.32, 0.4
#set xlabel "Time (min)" 
#set ylabel "Routing table poisoning" 
set ylabel
set label 11 "(d) 32 min epoch" at graph .5,.8 center
plot [-1:65] [0:1] "dats/32min_maelstrom_15mal_.dat" using ($1/60):2 title 'Maelstrom (15% Malicious)' w lp lt 1,\
"dats/churn-point-32.dat" using ($2/60):1 title 'Churn point' w lines lt 7,\
"dats/churn-point-32.dat" using ($3/60):1 notitle  w lines lt 7

 
set origin 1.10, 0.0
set size 0.32, 0.4
set xlabel "Time (min)" 
#set ylabel "Routing table poisoning" 
set ylabel
set label 11 "(e) Bamboo" at graph .5,.8 center
#set key bottom right
set nokey
plot [-1:65] [0:1] "dats/32min_bamboo_15mal_.dat" using ($1/60):2 title 'Bamboo (15% Malicious)' w lp lt 1

set nomultiplot

