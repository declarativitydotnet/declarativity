set terminal pdf
set ylabel "Runtime (sec)"
set xlabel "Number of paths"
plot "bench/perf.data" using 1:2 with lines, \
     "bench/perf.data" using 1:3 with lines, \
     "bench/perf.data" using 1:4 with lines
