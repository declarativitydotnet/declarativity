set term postscript eps monochrome dashed 13
set output "crtPoisoning.eps"
set size 0.7, 0.4

set multiplot


set data style lines
set origin 0.0,0.0 
set size 0.40, 0.4
#set grid 
set ylabel "CRT Poisoning (%)"
set xlabel "Time (min)" 
#set title "15% malicious fraction"
set nokey
plot [-1:65] [0:.5] "crt-15.dat" using ($2/60):1 title 'Maintenance Redundancy = 0',\
          "crt-15-red-16-2.dat" using ($1/60):2 title 'Maintenance Redundancy = 16',\
          0.15 notitle


set origin 0.35,0.0 
set size 0.35, 0.4
set data style linespoints
#set grid 
#set ylabel "CRT Poisoning"
set ylabel
set format y ""
set key top left
set xlabel "Malicious Presence (%)" 
plot [0:30] [0:.5] "crt.dat" using 1:($2/100) title 'No Redundancy',\
          "" using 1:($3/100) title 'Redundancy=16',\
	  x/100 title "Baseline"
set nomultiplot 
