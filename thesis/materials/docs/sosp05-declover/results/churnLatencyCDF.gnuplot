set term postscript eps color solid 12
set output "churnLatencyCDF.eps"

set size .45, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Latency (s)"
set ylabel "CDF"
set key bottom right

plot [0:4]	     	"churnLatencyCDF480.dat" using 1:($2/94829) title "8 min" with lines,\
			"churnLatencyCDF960.dat" using 1:($2/221385) title "16 min" with lines,\
			"churnLatencyCDF1920.dat" using 1:($2/156958) title "32 min" with lines,\
			"churnLatencyCDF3840.dat" using 1:($2/213104) title "64 min" with lines
			
