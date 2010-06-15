set terminal postscript eps 26
set output "message-counts-batching.ps"
set xlabel "f" 
set ylabel "Total messages/req"
set yrange [0:90]
set key left top
plot  "compare-msg-counts.dat" using 1:2 title "HQ" with linespoints, "compare-msg-counts.dat" using 1:5 title "PBFT" with linespoints, "hq-batching-msg-count.dat" using 1:2 title "HQ++ Batching 1" with linespoints, "hq-batching-msg-count.dat" using 1:3 title "HQ++ Batching 2" with linespoints, "hq-batching-msg-count.dat" using 1:4 title "HQ++ Batching 5" with linespoints, "hq-batching-msg-count.dat" using 1:5 title "HQ++ Batching 10" with linespoints
