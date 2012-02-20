set terminal pdf monochrome dashed
set ylabel "Runtime (sec)"
set xlabel "Number of paths"
plot "bench/perf.data" using 1:2 with lines title "Bloom", \
     "bench/perf.data" using 1:3 with lines title "Lattice (Semi-naive)", \
     "bench/perf.data" using 1:4 with lines title "Lattice (Naive)"
