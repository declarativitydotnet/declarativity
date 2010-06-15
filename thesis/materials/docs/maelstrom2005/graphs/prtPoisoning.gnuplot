set term postscript eps monochrome dashed 13
set output "prtPoisoning.eps"
set size .7, .5
set data style linespoints

set multiplot

set origin 0, 0.0
set size .37, .4
set xlabel "Malicious presence (%)" 0,.5
set ylabel "PRT Poisoning" 1
set key 10,1.35
set xtics ("5" 5, "10" 10, "15" 15, "20" 20, "25" 25)
set label 11 "(a)" at graph 0.45, 0.87
plot [0:30] [0:1] "combined2.dat" using 1:2 title '8 min',\
          "" using 1:3 title '16 min',\
          "" using 1:4 title '32 min',\
          "" using 1:5 title 'Bamboo'


set origin 0.35, 0.0 
set size 0.37, 0.4
#set grid 
set key 22,1.35
set ylabel "Prob. of Successful Routing" 1
set label 11 "(b)" at graph 0.45, 0.87
plot [0:30] [0:1] "paths.dat" using 1:4 title 'Bamboo: No redun.',\
          "" using 1:5 title 'Bamboo: Redun. 16',\
          "" using 1:2 title 'Maelstrom: No redun.',\
          "" using 1:3 title 'Maelstrom: Redun. 16'

set nomultiplot
