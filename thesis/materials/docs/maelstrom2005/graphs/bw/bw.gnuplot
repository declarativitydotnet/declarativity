set term postscript eps color dashed 18
set output "bw_window.eps"
#set term aqua

set xlabel "Time (secs)" 
set ylabel "Bandwidth (Bps)" 
plot [0:3000] [0:1000] "dat/window_0.dat" using 1:2 title 'Bamboo' w l lt 1, \
                       "dat/window_1000.dat" using 1:2 title 'Maelstrom (4 min.)' w l lt 2 lw 2, \
                       "dat/window_2000.dat" using 1:2 title 'Maelstrom (8 min.)' w l lt 3 lw 3, \
                       "dat/window_4000.dat" using 1:2 title 'Maelstrom (16 min.)' w l lt 4 lw 4, \
                       "dat/window_8000.dat" using 1:2 title 'Maelstrom (32 min.)' w l lt 5 lw 5, \
                       "dat/window_16000.dat" using 1:2 title 'Maelstrom (64 min.)' w l lt 6 lw 6, \
                       "dat/window_32000.dat" using 1:2 title 'Maelstrom (128 min.)' w l lt 7 lw 7, \
                       "dat/window_64000.dat" using 1:2 title 'Maelstrom (256 min.)' w l lt 8 lw 8
