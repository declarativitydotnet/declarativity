set term postscript eps monochrome dashed 13
set output "sybilPoisoning.eps"
set size 0.4, 0.4

set data style linespoints
set origin 0.0,0.0 
#set grid 
set ylabel "PRT Poisoning (%)"
set xlabel "Malicious presence (%)" 
#set nokey
set key 4,60 
set xtics ("1" 1, "3" 3, "5" 5)
plot [0:6] [0:100] "sybil.dat" using 1:2 title 'Maelstrom (32min)',\
          "" using 1:3 title 'Bamboo'


