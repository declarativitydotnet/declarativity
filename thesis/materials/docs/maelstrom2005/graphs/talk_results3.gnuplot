set term postscript eps monochrome dashed 13
set output "talk_results3.eps"
set size 1.0, 0.5

set data style linespoints

set multiplot

set origin 0.0, 0.0 
set size .5, .5
#set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
set xtics ("8" 2, "16" 3, "32" 4, "Bamb." 7)
set xlabel "Epoch Length (mins)" font "Helvetia, 18"
set ylabel "Latency (msec)" 1 font "Helvetia, 18"
set boxwidth .5
set style fill solid .5
plot [-1:8][0:510]  "latency.dat" using 0:2 notitle w boxes


set origin 0.5, 0.0 
set size .5, .5
#set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
set xtics ("8" 2, "16" 3, "32" 4, "Bamb." 7)
set xlabel "Epoch Length (mins)" font "Helvetia, 18"
set ylabel "Bandwidth (bytes/sec)" 1 font "Helvetia, 18"
set boxwidth .5
set style fill solid .5
plot [-1:8][0:4500]  "bw.dat" using 0:2 notitle w boxes

set nomultiplot 
