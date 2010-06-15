set term postscript eps
set output "manyJoins1.eps"
set tics out
set size .60, 0.4

set multiplot

set size 0.3, 0.4


set origin 0.0,0.0 

set nologscale xyx2y2
set xlabel "Number of Rules"
set ylabel "CPU Utilization %" 1
set key top right

plot [-5:260][0:] "manyJoins1.dat" using 1:2:($2-$3):($2+$3) with yerrorbars notitle



set origin 0.3, 0.0

set nologscale xyx2y2
set xlabel "Number of Rules"
set ylabel "Memory Utilization\n(MB)" 1
set key top left

plot [-5:260][0:] "manyJoins1.dat" using 1:($4/1000):(($4-$5)/1000):(($4+$5)/1000) with yerrorbars notitle

set nomultiplot
