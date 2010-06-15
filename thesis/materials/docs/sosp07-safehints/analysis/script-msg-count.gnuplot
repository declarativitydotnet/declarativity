set terminal postscript eps 26
set output "message-counts.ps"
set xlabel "f" 
set ylabel "Total messages/req"
set yrange [0:90]
set key left top
plot  "compare-msg-counts.dat" using 1:2 title "HQ" with linespoints, "compare-msg-counts.dat" using 1:3 title "HQ (UHG)" with linespoints, "compare-msg-counts.dat" using 1:4 title "HQ (Non-UHG)" with linespoints, "compare-msg-counts.dat" using 1:5 title "PBFT" with linespoints
