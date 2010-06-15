set term postscript eps 
set output "churnBW-400-new.eps"

set size .45, .5

set title ""
set nologscale yx2y2
set logscale x
set xtics 4,2
set xlabel "Session Time (min)"
set ylabel "Maintenance BW (Bytes/s)"
set key top right

plot [4:128][0:]	     	"node_bw_summary" using 1:2 notitle with linespoints
			
