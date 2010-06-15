set terminal postscript enhanced eps mono dashed 12
set output "ps-hq.eps"
set size .65, .9


set multiplot

set xrange [-.05:1.05]
set yrange [0:10000]

set origin 0, 0
set size .29, .32

set xtics rotate
set ylabel "Throughput\n(requests/s)" 1.5
set xlabel " "
set nokey

plot	"hq-f1-throughput.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f1-throughput.dat" using 1:($2/1) title "PS-HQ" with linespoints

set origin 0.24, 0
set size .22, .32

set xlabel "Contention" 
set noylabel
set format y ""
set nokey

plot	"hq-f2-throughput.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f2-throughput.dat" using 1:($2/1) title "PS-HQ" with linespoints

set origin 0.41, 0
set size .22, .32

set xlabel " "
set noylabel
set nokey

plot	"hq-f3-throughput.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f3-throughput.dat" using 1:($2/1) title "PS-HQ" with linespoints











##################################################
# Latency
##################################################

set yrange [0:0.05]
set format x ""

set size .29, .27
set origin 0, .3

set xtics rotate
set ylabel "Latency\n(s)" .9
set xlabel " "
set nokey
set format y

plot	"hq-f1-latency.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f1-latency.dat" using 1:($2/1) title "PS-HQ" with linespoints


set origin 0.24, .3
set size .22, .27

set xlabel " " 
set noylabel
set format y ""
set nokey

plot	"hq-f2-latency.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f2-latency.dat" using 1:($2/1) title "PS-HQ" with linespoints

set origin 0.41, .3
set size .22, .27

set xlabel " "
set noylabel
set nokey

plot	"hq-f3-latency.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f3-latency.dat" using 1:($2/1) title "PS-HQ" with linespoints








##################################################
# Traffic
##################################################

set yrange [0:4]
set format x ""

set origin 0, .55
set size .29, .27

set xtics rotate
set ylabel "Traffic\n(KB/req)" .2 
set xlabel " "
set key .5, 2
set format y

set label 100 "f = 1" at .5, 5 center font "HelveticaBold,16"
plot	"hq-f1-traffic.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f1-traffic.dat" using 1:($2/1) title "PS-HQ" with linespoints


set origin 0.24, .55
set size .22, .27

set xlabel " " 
set noylabel
set format y ""
set nokey

set label 100 "f = 2" at .5, 5 center
plot	"hq-f2-traffic.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f2-traffic.dat" using 1:($2/1) title "PS-HQ" with linespoints

set origin 0.41, .55
set size .22, .27

set xlabel " "
set noylabel
set nokey

set label 100 "f = 3" at .5, 5 center
plot	"hq-f3-traffic.dat" using 1:($2/1) title "HQ" with linespoints, \
	"ps-hq-f3-traffic.dat" using 1:($2/1) title "PS-HQ" with linespoints












set nomultiplot