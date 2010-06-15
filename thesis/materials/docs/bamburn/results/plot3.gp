set term postscript eps
set output "$0.eps"               
set xlabel "Time (seconds)"
set ylabel "% Route table entries poisoned"

set origin 0,0 
set size 0.5, 0.5
plot [0:1000.0] [0:100.0] "$0.dat" using 1:2 w lp title '8.5% malicious',"$0.dat" using 1:3 w lp title '16.5% malicious',"$0.dat" using 1:4 w lp title '25% malicious'

