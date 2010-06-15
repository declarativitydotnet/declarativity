set term postscript eps
set output "snapshots.eps"
set size .53, 0.64
set ytics nomirror
set tics out

set multiplot

set size 0.29, 0.29

set origin 0.0,0.37

set nologscale xyx2y2
#set logscale x
set xtics nomirror rotate ("" -0.125, "" 0.03125,  "" 0.25, "" 0.5, "" 0.75, "" 1)
set xlabel ""
set ylabel "CPU Utilization %" 
set key top right

plot [-.25:1.1][0:] "snapshots.dat" using (1/$1):2:($2-$3):($2+$3) with yerrorbars notitle

set size 0.29, 0.4
set origin 0.0,0

set nologscale xyx2y2
#set logscale x
set xtics nomirror rotate ("None" -0.125, "1/32" 0.03125,  "1/4" 0.25, "1/2" 0.5, "3/4" 0.75, "1" 1)
set xlabel "Rate (1/sec)" ,-.75
set ylabel "Tx Messages\n(x1000)" 1
set key top right

plot [-.25:1.1][0:6.7] "snapshots.Tuples.dat" using (1/$1):($2/1000):($2/1000-$3/1000):($2/1000+$3/1000) with yerrorbars notitle
			

set size 0.3, 0.29

set origin 0.25, 0.37

set nologscale xyx2y2
#set logscale x
set xtics nomirror rotate ("" -0.125, "" 0.03125,  "" 0.25, "" 0.5, "" 0.75, "" 1)
set xlabel ""
set ylabel "Process Memory\n(MB)"  1
set key top left

plot [-.25:1.1][0:] "snapshots.dat" using (1/$1):($4/1000):($4/1000-$5/1000):($4/1000+$5/1000) with yerrorbars notitle

set size 0.3, 0.4

set origin 0.25, 0.0

set nologscale xyx2y2
#set logscale x
set xtics nomirror rotate ("None" -0.125, "1/32" 0.03125,  "1/4" 0.25, "1/2" 0.5, "3/4" 0.75, "1" 1)
set xlabel "Rate (1/sec)" ,-.75
set ylabel "Live Tuples\n(1000x)"  1.7
set key top left

plot [-.25:1.1][0:2.9] "snapshots.Tuples.dat" using (1/$1):($8/1000):($8/1000-$9/1000):($8/1000+$9/1000) with yerrorbars notitle

set nomultiplot
