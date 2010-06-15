set term postscript mono eps
set output "safeInterior.eps"
set size 0.62, 0.35
set style data lines
set nokey

set multiplot

set origin 0, 0
set size 0.34, .37
set xlabel "Predicate selectivity"
set ylabel "Biased estimate\n(e=0.1)" 1.3
plot[-.1:1.1][-.1:1.1] "munged.safeInterior.FM.0.10.Cov1562.dat" using ($2/100000):($10/100000) with lines lw 2 title "1/64",\
		"munged.safeInterior.FM.0.10.Cov6250.dat" using ($2/100000):($10/100000) with lines lw 2 title "1/16",\
		"munged.safeInterior.FM.0.10.Cov25000.dat" using ($2/100000):($10/100000) with lines lw 2 title "1/4",\
		"munged.safeInterior.FM.0.10.Cov100000.dat" using ($2/100000):($10/100000) with lines lw 2 title "Full",\
		x title "X=Y"

set origin 0.30, 0
set xlabel "Predicate selectivity"
set ylabel "Biased estimate\n(e=0.25)" 1.3
set key left
plot[-.1:1.1][-.1:1.1] "munged.safeInterior.FM.0.25.Cov1562.dat" using ($2/100000):($10/100000) with lines lw 2 title "1/64",\
		"munged.safeInterior.FM.0.25.Cov6250.dat" using ($2/100000):($10/100000) with lines lw 2 title "1/16",\
		"munged.safeInterior.FM.0.25.Cov25000.dat" using ($2/100000):($10/100000) with lines lw 2 title "1/4",\
		"munged.safeInterior.FM.0.25.Cov100000.dat" using ($2/100000):($10/100000) with lines lw 2 title "Full",\
		x title "X=Y"

set nomultiplot
