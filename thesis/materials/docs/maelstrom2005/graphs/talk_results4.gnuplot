set term postscript eps color dashed 18
set output "talk_results4.eps"
set size 1.0, .7
set data style linespoints

set multiplot

set origin 0, 0.0
set size 0.5, 0.5
set xlabel "Malicious presence (%)" 0,.5
set ylabel "PRT Poisoning" 1
set key 18,1.5
set xtics ("5" 5, "10" 10, "15" 15, "20" 20, "25" 25)
plot [0:30] [0:1] "combined2.dat" using 1:2 title '8 min',\
          "" using 1:3 title '16 min',\
          "" using 1:4 title '32 min',\
          "" using 1:5 title 'Bamboo',\
          "" using 1:6 title 'Malicious Fraction' 


set origin 0.5, 0.0 
set size 0.5, 0.5
#set grid 
set key 22,1.5
set ylabel "Prob. of Successful Routing" 1
plot [0:30] [0:1] "paths.dat" using 1:4 title 'Bamboo: No redun.',\
          "" using 1:5 title 'Bamboo: Redun. 16',\
          "" using 1:2 title 'Maelstrom: No redun.',\
          "" using 1:3 title 'Maelstrom: Redun. 16' 

set nomultiplot
