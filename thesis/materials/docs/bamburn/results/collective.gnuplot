set term postscript eps color dashed 14
set output "collective.eps"
set size 1.4, .5
set data style linespoints

set multiplot
set origin 0,0 
set size 0.46, 0.5
set xlabel "Session Time (seconds)"
set log x
set xtics ("30" 30, "60" 60, "120" 120, "240" 240, "480" 480, "960" 960)

set ylabel "% of Consistent Lookups"
set key bottom right
plot [29:1000.0] [0:110.0] "graph1.dat" using 1:2 title 'No precomputation',\
	"graph2.dat" using 1:2 title 'Prospective table'

set origin 0.46, 0
set xlabel "Session Time (seconds)"
set ylabel "Lookup Latency (ms)"
set key bottom left
plot [29:1000] [0:300] "graph1.dat" using 1:3 title "No precomputation",\
	"graph2.dat" using 1:3 title "Prospective table"


set origin 0.92,0
set key top right
set ylabel "Level of Poisoning (%)"
plot [29:1000.0] [0:100.0] "graph4.dat" using 1:2 title '8.5% malicious' w lp lt 1,\
	"" using 1:3 title '16.5% malicious' w lp lt 4,\
	"" using 1:4 title '25% malicious' w lp lt 3,\
	"graph3.dat" using 1:(42) notitle w lines lt 1 ,\
	"" using 1:(60) notitle w lines lt 4,\
	"" using 1:(70) notitle w lines lt 3


set nomultiplot

