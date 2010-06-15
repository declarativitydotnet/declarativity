set term postscript eps 
set output "noChurnBW.eps"

set size .45, .5

set title ""
set nologscale yx2y2
#set logscale x
#set xtics 128,2
set xlabel "Population Size"
set ylabel "Maintenance BW (Bytes/s)"
set key top right

plot [0:] [0:] "node_bw_summary" using 1:2 notitle with linespoints
			
