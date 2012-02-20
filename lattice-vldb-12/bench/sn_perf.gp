set terminal pdfcairo monochrome dashed font "Helvetica,9" linewidth 4
set ylabel "Runtime (sec)"
set xlabel "Graph size (number of paths)"
set key top left
plot "bench/bloom.data" using 3:4 with lines title "Bloom", \
     "bench/seminaive-lat.data" using 3:4 with lines title "Lattice (Semi-Naive)", \
     "bench/naive-lat.data" using 3:4 with lines title "Lattice (Naive)"
