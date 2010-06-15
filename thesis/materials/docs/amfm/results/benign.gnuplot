set term postscript mono eps
set output "benign.eps"
set size 0.62, 0.60
set nokey

set multiplot

set origin 0, 0
set size 0.34, .35
set xlabel "Predicate selectivity"
set ylabel "Total Alarms frac." 1.3
plot[-.1:1.1][0:.1] 	"benignFPFN0.1.dat" using ($4/100000):($8/$6) title "0.1" with linespoints,\
			"benignFPFN0.12.dat" using ($4/100000):($8/$6) title "0.12" with linespoints,\
			"benignFPFN0.14.dat" using ($4/100000):($8/$6) title "0.14" with linespoints,\
			"benignFPFN0.16.dat" using ($4/100000):($8/$6) title "0.16" with linespoints,\
			"benignFPFN0.18.dat" using ($4/100000):($8/$6) title "0.18" with linespoints,\
			"benignFPFN0.2.dat" using ($4/100000):($8/$6) title "0.2" with linespoints,\
			"benignFPFN0.22.dat" using ($4/100000):($8/$6) title "0.22" with linespoints


set origin 0.30, 0
set xlabel "Predicate selectivity"
set ylabel "Out-of-bounds frac."
plot[-.1:1.1][0:.1] 	"benignFPFN0.1.dat" using ($4/100000):(1-$10/$6) title "0.1" with linespoints,\
			"benignFPFN0.12.dat" using ($4/100000):(1-$10/$6) title "0.12" with linespoints,\
			"benignFPFN0.14.dat" using ($4/100000):(1-$10/$6) title "0.14" with linespoints,\
			"benignFPFN0.16.dat" using ($4/100000):(1-$10/$6) title "0.16" with linespoints,\
			"benignFPFN0.18.dat" using ($4/100000):(1-$10/$6) title "0.18" with linespoints,\
			"benignFPFN0.2.dat" using ($4/100000):(1-$10/$6) title "0.2" with linespoints,\
			"benignFPFN0.22.dat" using ($4/100000):(1-$10/$6) title "0.22" with linespoints



set origin 0.30, 0.32
set size 0.34, .30
set noxlabel
set noxtics
set ylabel "False negatives frac."
plot[-.1:1.1][0:.1] 	"benignFPFN0.1.dat" using ($4/100000):($14/$6) title "0.1" with linespoints,\
			"benignFPFN0.12.dat" using ($4/100000):($14/$6) title "0.12" with linespoints,\
			"benignFPFN0.14.dat" using ($4/100000):($14/$6) title "0.14" with linespoints,\
			"benignFPFN0.16.dat" using ($4/100000):($14/$6) title "0.16" with linespoints,\
			"benignFPFN0.18.dat" using ($4/100000):($14/$6) title "0.18" with linespoints,\
			"benignFPFN0.2.dat" using ($4/100000):($14/$6) title "0.2" with linespoints,\
			"benignFPFN0.22.dat" using ($4/100000):($14/$6) title "0.22" with linespoints

set origin 0, 0.32
set noxlabel
set ylabel "False positives frac."
set key default
plot[-.1:1.1][0:.1] 	"benignFPFN0.1.dat" using ($4/100000):($12/$6) title "0.1" with linespoints,\
			"benignFPFN0.12.dat" using ($4/100000):($12/$6) title "0.12" with linespoints,\
			"benignFPFN0.14.dat" using ($4/100000):($12/$6) title "0.14" with linespoints,\
			"benignFPFN0.16.dat" using ($4/100000):($12/$6) title "0.16" with linespoints,\
			"benignFPFN0.18.dat" using ($4/100000):($12/$6) title "0.18" with linespoints,\
			"benignFPFN0.2.dat" using ($4/100000):($12/$6) title "0.2" with linespoints,\
			"benignFPFN0.22.dat" using ($4/100000):($12/$6) title "0.22" with linespoints


set nomultiplot
