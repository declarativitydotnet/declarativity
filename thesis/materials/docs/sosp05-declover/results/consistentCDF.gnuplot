set term postscript eps color solid 12
set output "consistentCDF.eps"

set size .45, .5

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Consistent fraction"
set ylabel "CDF"
set key bottom right

set arrow 11 from graph 0.5, 1 to graph 0.5, 0 nohead lt 4
set label 12 "Consistency threshold" at graph 0.5, 0.5 center rotate

plot [0:1]	     	"consistentCDF480.dat" using 1:($2/102) title "8 min" with lines,\
			"consistentCDF960.dat" using 1:($2/261) title "16 min" with lines,\
			"consistentCDF1920.dat" using 1:($2/261) title "32 min" with lines,\
			"consistentCDF3840.dat" using 1:($2/270) title "64 min" with lines
			
