set term postscript eps color solid 12
set output "noChurnLatencyCDF.eps"

set size .45, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Latency (s)"
set ylabel "CDF"
set key top right

plot [0:4]	     	"noChurnLatencyCDF100.dat" using 1:($2/15874) title "100 nodes" with lines,\
			"noChurnLatencyCDF500.dat" using 1:($2/30383) title "500 nodes" with lines
			
