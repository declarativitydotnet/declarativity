set term postscript eps color solid 18
set output "hop_lookup_dynamic.eps"

set size 1., 1. 

set title ""
set xtics ("0" 0, "2" 120, "4" 240, "6" 360, "8" 480, "10" 600, "12" 720, "14" 840, "16" 960, "18" 1080, "20" 1200, "22" 1320, "24" 1440, "26" 1560, "28" 1680, "30" 1800, "32" 1920, "34" 2040, "36" 2160, "38" 2280)
set xlabel "Time (minutes)" font "Helvetia, 22"
set ylabel "Mean hop count" font "Helvetia, 22"
set nokey

plot [-0.5:] "hops21dynamic.dat" using 1:2:3 w yerrorlines
