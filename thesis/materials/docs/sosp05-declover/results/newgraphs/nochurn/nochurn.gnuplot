set term postscript eps
set output "nochurn.eps"
set size 1.35, 0.5

set multiplot


set origin 0.0,0.0 
set size 0.45, 0.5

set nologscale xyx2y2
set tics out
set xlabel "Hop Count"
set ylabel "Frequency" 1, 0
set key top right

set title "(i)"
plot [-0.5:16.5] "simple_latency_hopdist-100.dat" using 1:3 title \
	     "100" with linespoints,\
	     "simple_latency_hopdist-300.dat" using 1:3 title \
	     "300" with linespoints,\
	     "simple_latency_hopdist-500.dat" using 1:3 title \
	     "500" with linespoints 
			

set origin 0.46, 0.0

set title "(ii)"
set nologscale yx2y2
set xlabel "Population Size"
set ylabel "Maintenance BW (Bytes/s)"
set key top right

plot [20:580] [0:] "node_bw_summary" using 1:2 notitle with linespoints


set origin 0.9, 0.0

set title "(iii)"
set nologscale xyx2y2
set tics out
set xlabel "Latency (s)"
set ylabel "CDF" 1.5, 0
set key bottom right

plot [0:10][0:1.05] "simple_latency_cdf-100.dat" using 1:($3/100) title "100" \
     with lines linewidth 2, \
     "simple_latency_cdf-300.dat" using 1:($3/100) title "300" \
     with lines linewidth 2, \
     "simple_latency_cdf-500.dat" using 1:($3/100) title "500" \
     with lines linewidth 2

set nomultiplot
