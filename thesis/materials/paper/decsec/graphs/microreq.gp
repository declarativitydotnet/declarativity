set term postscript mono dashed eps enhanced 13
set output 'microreq.eps'
set xlabel 'Request size (KB)'
set ylabel 'Processing time (ms)' 1.2
set xrange [-.5:4.5]
set yrange [0:]

set size .65, .45
 
set pointsize 1
 

set multiplot

set origin 0,0
set size .36, .45
set nokey

plot 'req.dat' u ($1/1000):5 t 'A2M-PBFT-A(sig)' w lp, \
'req.dat' u ($1/1000):3 t 'A2M-PBFT(sig)' w lp, \
'req.dat' u ($1/1000):6 t 'A2M-PBFT-A(mac)' w lp, \
'req.dat' u ($1/1000):4 t 'A2M-PBFT(mac)' w lp, \
'req.dat' u ($1/1000):2 t 'PBFT' w lp


set origin 0.32, 0
set size .32, .45

set key 3,3.8
set xlabel 'Response size (KB)'
set ylabel ""
set format y ""

plot 'resp.dat' u ($1/1000):5 t 'A2M-PBFT-A(sig)' w lp, \
'resp.dat' u ($1/1000):3 t 'A2M-PBFT(sig)' w lp, \
'resp.dat' u ($1/1000):6 t 'A2M-PBFT-A(MAC)' w lp, \
'resp.dat' u ($1/1000):4 t 'A2M-PBFT(MAC)' w lp, \
'resp.dat' u ($1/1000):2 t 'PBFT' w lp
