set term postscript eps monochrome dashed 13
set output "bwneighbors.eps"
set size .56, .4
set data style linespoints

set multiplot

set origin 0.0, 0.0 
set size .31, .4
set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
set xlabel "Epoch Length (mins)" 
set ylabel "Bandwidth (bytes/sec)" 1
set boxwidth .5
set style fill solid .5
set label 11 "(d)" at graph .5, .8 center
plot [-1:8][0:900]  "bandwidth.dat" using 0:2 notitle w boxes

set origin 0.28, 0.0 
set size .30, .4
set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
set xlabel "Epoch Length (mins)" 
set ylabel "Average Neighbors" 1
set boxwidth .5
set style fill solid .5
set label 11 "(e)" at graph .5, .8 center
plot [-1:8][0:40]  "neighbor.dat" using 0:2 notitle w boxes

set nomultiplot



