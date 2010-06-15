set term postscript eps monochrome dashed 13
set output "simPerformance.eps"
set size 0.84, 0.4

set data style linespoints

set multiplot


set origin 0.0, 0.0 
set size .31, .4
set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
#set xtics ("8" 0, "16" 1, "32" 2, "Bamb." 4)
set xlabel "Epoch Length (mins)" 
set ylabel "Latency (msec)" 1
set boxwidth .5
set style fill solid .5
set label 11 "(a)" at graph .5, .8 center
plot [-1:8][0:510]  "latency.dat" using 0:2 notitle w boxes


#set origin 0.27, 0.0 
#set size .31, .4
#set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
#set xtics ("8" 0, "16" 1, "32" 2, "Bamb." 4)
#set xlabel "Epoch Length (mins)" 
#set ylabel "Hopcount" 1.5
#set boxwidth .5
#set style fill solid .5
#set label 11 "(b)" at graph .5, .8 center
#plot [-1:5][0:10]  "hopcount.dat" using 0:2 notitle w boxes


set origin 0.28, 0.0 
set size .31, .4
set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
#set xtics ("8" 0, "16" 1, "32" 2, "Bamb." 4)
set xlabel "Epoch Length (mins)" 
set ylabel "Bandwidth (bytes/sec)" 1
set boxwidth .5
set style fill solid .5
set label 11 "(b)" at graph .5, .8 center
plot [-1:8][0:4500]  "bw.dat" using 0:2 notitle w boxes

set origin 0.56, 0.0 
set size .31, .4
set xtics ("2" 0, "4" 1, "8" 2, "16" 3, "32" 4, "64" 5, "Bamb." 7)
#set xtics ("8" 0, "16" 1, "32" 2, "Bamb." 4)
set xlabel "Epoch Length (mins)" 
set ylabel "Bandwidth (bytes/sec)" 1
set boxwidth .5
set style fill solid .5
set label 11 "(c)" at graph .5, .8 center
plot [-1:8][0:900]  "bw_500_alg_sim.dat" using 0:2 notitle w boxes



set nomultiplot 
