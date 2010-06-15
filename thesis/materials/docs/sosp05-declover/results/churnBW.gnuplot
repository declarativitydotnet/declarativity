set term postscript eps color solid 12
set output "churnBW.eps"

set size .45, .5

set title ""
set nologscale yx2y2
set logscale x
set xtics 4,2
set xlabel "Session Time (min)"
set ylabel "Maintenance BW (Bytes/s)"
set key top right

plot [4:128][0:]	     	"churnBW.dat" using ($1/60):4 notitle with linespoints
			
