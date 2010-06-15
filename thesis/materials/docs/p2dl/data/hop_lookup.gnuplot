set term postscript eps color solid 18
set output "hop_lookup.eps"

set size 1., 1. 

set title ""
set nologscale xyx2y2
set tics out
set xlabel "Hop Count" font "Helvetia, 22"
set ylabel "Frequency" font "Helvetia, 22"
set key top right

plot [-0.5:] "hops21finger.dat" using 2:($1/529.0) title "Successor + fingers" with linespoints,\
             "hops21succ.dat" using 2:($1/923.0) title "Successor (no fingers)" with linespoints
