set term postscript eps color dashed 13
#set term tgif dashed "Helvetia" 20
set output "poisoning.eps"
set data style linespoints

set size .6, .4
set xlabel "Malicious fraction in population" 
set ylabel "Routing Table Poisoning" 
set key 31,0.74 
set xtics ("5" 5, "10" 10, "15" 15, "20" 20, "25" 25)
plot [0:38] [0:1] "combined2.dat" using 1:2 title 'Maelstrom (8min)' w lp lt 1,\
          "" using 1:3 title '(16min)' w lp lt 2,\
          "" using 1:4 title '(32min)' w lp lt 3,\
          "" using 1:5 title 'Bamboo' w lp lt 4

