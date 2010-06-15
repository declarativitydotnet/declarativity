set terminal postscript enhanced eps mono dashed 12
set output "pbft.eps"
set size .65, .9

set log x

set multiplot

set xrange [.5:300]
set xtics (1, 5, 25, 120)
set yrange [0:15000]

set origin 0, 0
set size .29, .32

set xtics rotate
set ylabel "Throughput\n(requests/s)" 1.5
set xlabel " "
set nokey

plot	"pbft-f1-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
	"pbft-f1-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
	"pbft-f1-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 100" with linespoints

set origin 0.24, 0
set size .22, .32

#set xtics (1, 5, 25, 120)
set xlabel "Number of clients" 
set noylabel
set format y ""
set nokey

plot	"pbft-f2-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
	"pbft-f2-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
	"pbft-f2-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 100" with linespoints

set origin 0.41, 0
set size .22, .32

set xlabel " "
set noylabel
set nokey

plot    "pbft-f3-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f3-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f3-beta-10-Throughput.dat" using 1:($2/1) title "Batchsize = 100" with linespoints


##################################################
# Latency
##################################################

set yrange [0:0.03]
set format x ""

set size .29, .27
set origin 0, .3

set xtics rotate
set ylabel "Latency\n(s)" .9
set xlabel " "
set nokey
set format y

plot    "pbft-f1-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f1-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f1-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 100" with linespoints



set origin 0.24, .3
set size .22, .27

set xlabel " " 
set noylabel
set format y ""
set nokey

plot    "pbft-f2-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f2-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f2-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 100" with linespoints




set origin 0.41, .3
set size .22, .27

set xlabel " "
set noylabel
set nokey

plot    "pbft-f3-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f3-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f3-beta-10-latency.dat" using 1:($2/1) title "Batchsize = 100" with linespoints









##################################################
# Traffic
##################################################

set yrange [0:3]
set format x ""

set origin 0, .55
set size .29, .27

set xtics rotate
set ylabel "Traffic\n(KB/req)" .2 
set xlabel " "
set key 70, 1.5
set format y

set label 100 "f = 1" at 11, 2.5 center font "HelveticaBold,16"

plot    "pbft-f1-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f1-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f1-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 100" with linespoints



set origin 0.24, .55
set size .22, .27

set xlabel " " 
set noylabel
set format y ""
set nokey

set label 100 "f = 2" at 11, 2.5 center

plot    "pbft-f2-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f2-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f2-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 100" with linespoints


set origin 0.41, .55
set size .22, .27

set xlabel " "
set noylabel
set nokey

set label 100 "f = 3" at 11, 2.5 center
plot    "pbft-f3-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 10" with linespoints, \
        "pbft-f3-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 40" with linespoints, \
        "pbft-f3-beta-10-traffic.dat" using 1:($2/1) title "Batchsize = 100" with linespoints













set nomultiplot
