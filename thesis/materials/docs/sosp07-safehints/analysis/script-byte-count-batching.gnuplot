set terminal postscript eps 26
set output "byte-counts-Payload-256B-Batching.ps"
set xlabel "f" 
set ylabel "Total Kbytes/req"
set yrange [0:14]
set key left top
plot "compare-bytes-10-byte-req-batching.dat" using 1:2 title "HQ" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:2 title "HQ++ Batching 1" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:3 title "HQ++ Batching 2" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:4 title "HQ++ Batching 5" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:5 title "HQ++ Batching 10" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:6 title "PBFT Batching 1" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:7 title "PBFT Batching 2" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:8 title "PBFT Batching 5" with linespoints, \
	"compare-bytes-10-byte-req-batching.dat" using 1:9 title "PBFT Batching 10" with linespoints
