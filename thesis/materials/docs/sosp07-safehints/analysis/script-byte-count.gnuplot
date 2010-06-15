set terminal postscript eps 26
set output "byte-counts-Payload-256B.ps"
set xlabel "f" 
set ylabel "Total Kbytes/req"
set yrange [0:14]
set key left top
plot  "compare-req-256bytes.dat" using 1:2 title "HQ" with linespoints, "compare-req-256bytes.dat" using 1:3 title "HQ (UHG)" with linespoints, "compare-req-256bytes.dat" using 1:4 title "HQ (Non-UHG)" with linespoints, "compare-req-256bytes.dat" using 1:5 title "PBFT" with linespoints
