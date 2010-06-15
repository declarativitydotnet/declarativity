set term postscript eps
set output "manyPeriodics10.eps"
set size .70, 0.4
set xtics 1,2

set multiplot

set size 0.35, 0.4


set origin 0.0,0.0 

set nologscale yx2y2
set logscale x
set tics in
set xlabel "Number of Rules"
set ylabel "CPU Utilization"
set key top right

plot [0.25:15][] "manyPeriodics10.dat" using 1:2:($2-$3):($2+$3) with yerrorbars notitle
			

set origin 0.35, 0.0

set nologscale yx2y2
set logscale x
set tics in
set xlabel "Number of Rules"
set ylabel "Memory Utilization (KBytes)"
set key top left

plot [0.25:15][] "manyPeriodics10.dat" using 1:4:($4-$5):($4+$5) with yerrorbars notitle

set nomultiplot
