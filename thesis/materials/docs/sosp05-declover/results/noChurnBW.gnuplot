set term postscript eps color solid 12
set output "noChurnBW.eps"

set size .45, .5

set title ""
set nologscale yx2y2
set logscale x
set xtics 128,2
set xlabel "Population Size"
set ylabel "Maintenance BW (Bytes/s)"
set key top right

plot [90:][0:]	     	"noChurnBW.dat" using 1:3 notitle with linespoints
			
