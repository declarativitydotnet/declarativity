set size .65, .98
set terminal postscript port enhanced eps monochrome dashed  "Times-Roman" 14
set output 'magicNumbers.eps'
set key left top
set yrange [0:29]
set xrange [0.5:10.5]
set nologscale x
#set pointsize 0.5
set style data linespoints


set multiplot 
set origin 0, .65
set size .65, .35
set ylabel "Tuples received" 1
set noxlabel
set xtics 1
set format x ""

set boxwidth .3
plot	"nomagic.dat" using ($1+.8):2 title "Baseline" with boxes fill pattern 1, \
	"magic.dat" using ($1+1.2):2 title "Magic sets" with boxes fill pattern 3
	

set origin 0, .35
set size .65, .35
set ylabel "Tuples sent" 1
set noxlabel
set format x ""

set boxwidth .3
plot	"nomagic.dat" using ($1+.8):3 title "Baseline" with boxes fill pattern 1, \
	"magic.dat" using ($1+1.2):3 title "Magic sets" with boxes fill pattern 3
	
set origin 0, 0
set size .65, .4
set ylabel "Tuples generated" 1
set xlabel "Node ID" 0,0

set format x "%g"

set boxwidth .3
plot	"nomagic.dat" using ($1+.8):4 title "Baseline" with boxes fill pattern 1, \
	"magic.dat" using ($1+1.2):4 title "Magic sets" with boxes fill pattern 3
	
