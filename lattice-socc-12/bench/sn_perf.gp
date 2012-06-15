set terminal pdfcairo font "Helvetica,18" linewidth 4 rounded
set ylabel "Runtime (sec)"
set xlabel "Graph size (number of paths, 1000s)"
set key top right

set style line 1 lt rgb "#A00000" lw 2 pt 1
set style line 2 lt rgb "#00A000" lw 2 pt 6
set style line 3 lt rgb "#5060D0" lw 2 pt 2
set style line 4 lt rgb "#F25900" lw 2 pt 9

plot "bench/bloom.summary" using ($3/1000):4 title "Bloom" w lp ls 1, \
     "bench/seminaive-lat.summary" using ($3/1000):4 title "Lattice (S-Naive)" w lp ls 2, \
     "bench/naive-lat.summary" using ($3/1000):4 title "Lattice (Naive)" w lp ls 3
