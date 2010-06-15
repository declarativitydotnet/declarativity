set term postscript eps color solid 12
set output "hop_lookup.eps"

set size .45, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Hop Count"
set ylabel "Frequency"
set key top right

plot [-0.5:]     	"hop_lookup_128.dat" using 1:($2/$3) title "128 nodes" with linespoints,\
			"hop_lookup_192.dat" using 1:($2/$3) title "192 nodes" with linespoints,\
			"hop_lookup_256.dat" using 1:($2/$3) title "256 nodes" with linespoints,\
			"hop_lookup_384.dat" using 1:($2/$3) title "384 nodes" with linespoints,\
			"hop_lookup_512.dat" using 1:($2/$3) title "512 nodes" with linespoints
			
