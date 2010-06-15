set term postscript eps
set output "churn.eps"
set size 1.35, 0.5

set multiplot


set origin 0.0,0.0 
set size 0.45, 0.5

set title "(i)"
set nologscale yx2y2
set logscale x
set xtics 4,2
set xlabel "Session Time (min)"
set ylabel "Maintenance BW (Bytes/s)" 1,0
set key top right

plot [5:200][0:]	     	"node_bw_summary" using 1:2 notitle with linespoints
			

set origin 0.44, 0.0

set title "(ii)"
set nologscale xyx2y2
set tics out
set xlabel "Consistent fraction"
set ylabel "CDF"
set xtics 0, .2
set key top left

set arrow 11 from graph 0.5, 1 to graph 0.5, 0 nohead lt 4
set label 12 "Consistency threshold" at graph 0.5, 0.5 center rotate

plot [0:1.05][0:1.05] "consistency-8.dat" using 3:4 title "8 min" with lines linewidth 2, \
     "consistency-16.dat" using 3:4 title "16 min" with lines linewidth 2, \
     "consistency-32.dat" using 3:4 title "32 min" with lines linewidth 2, \
    "consistency-64.dat" using 3:4 title "64 min" with lines linewidth 2, \
     "consistency-128.dat" using 3:4 title "128 min" with lines linewidth 2 \

set nolabel 12
set noarrow 11

set origin 0.9, 0.0

set title "(iii)"
set nologscale xy
set tics out
set xlabel "Latency (s)"
set ylabel "CDF"
set xtics 0, 2
set ytics 0, .2
set key bottom right

plot [0:20][0:1.05] "simple_latency_cdf-8.dat" using 1:($3/100) title "8 min" with lines linewidth 2, \
   "simple_latency_cdf-16.dat" using 1:($3/100) title "16 min" with lines linewidth 2, \
   "simple_latency_cdf-32.dat" using 1:($3/100) title "32 min" with lines linewidth 2, \
     "simple_latency_cdf-64.dat" using 1:($3/100) title "64 min" with lines linewidth 2, \
     "simple_latency_cdf-128.dat" using 1:($3/100) title "128 min" \
     with lines linewidth 2


set nomultiplot
