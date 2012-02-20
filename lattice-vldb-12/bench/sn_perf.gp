set terminal pdfcairo monochrome dashed font "Helvetica,9" linewidth 4# rounded
set ylabel "Runtime (sec)"
set xlabel "Graph size (number of paths)"
set key top left
plot "bench/perf.data" using 1:2 with lines title "Bloom", \
     "bench/perf.data" using 1:3 with lines title "Lattice (Semi-Naive)", \
     "bench/perf.data" using 1:4 with lines title "Lattice (Naive)"
