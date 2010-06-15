set term postscript mono eps
set output "targeted.eps"
set size 1.37, 0.78
set style data lines 

set nokey

set multiplot



set size 0.30, .28
set origin 0, .52
set noxlabel
set ylabel "" 1.3
plot [-.01:0.21][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y20000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y20000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y20000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y20000.dat" using ($4/100000):($10/100000) with lines lw 2 title "full"


set origin 0.27, .52
set ylabel "" 1.3
plot[-.01:.21][-.1:1.1] "munged.targeted.FM.0.15.Cov1562.Y20000.dat" using ($4/100000):16 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.15.Cov6250.Y20000.dat" using ($4/100000):16 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.15.Cov25000.Y20000.dat" using ($4/100000):16 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.15.Cov100000.Y20000.dat" using ($4/100000):16 with lines lw 2 title "full"

set origin 0.54, .52
set key top title "Coverage"
set ylabel "" 1.3
plot[-.01:.21][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y20000.dat" using ($4/100000):16 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y20000.dat" using ($4/100000):16 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y20000.dat" using ($4/100000):16 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y20000.dat" using ($4/100000):16 with lines lw 2 title "full"
set nokey

set origin 0.81, .52
set ylabel "" 1.3
plot[-.01:.21][-.1:1.1] "munged.targeted.FM.0.15.Cov1562.Y20000.dat" using ($4/100000):12 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.15.Cov6250.Y20000.dat" using ($4/100000):12 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.15.Cov25000.Y20000.dat" using ($4/100000):12 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.15.Cov100000.Y20000.dat" using ($4/100000):12 with lines lw 2 title "full"

set origin 1.08, .52
set ylabel "" 1.3
plot[-.01:.21][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y20000.dat" using ($4/100000):12 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y20000.dat" using ($4/100000):12 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y20000.dat" using ($4/100000):12 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y20000.dat" using ($4/100000):12 with lines lw 2 title "full"








set origin 0, .27
set ylabel "Suppressed selectivity estimate" 1.3
plot [-.01:0.51][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y50000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y50000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y50000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y50000.dat" using ($4/100000):($10/100000) with lines lw 2 title "full"


set origin 0.27, .27
set ylabel "Alarm Fraction (e=0.15)" 1.3
plot[-.01:.51][-.1:1.1] "munged.targeted.FM.0.15.Cov1562.Y50000.dat" using ($4/100000):16 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.15.Cov6250.Y50000.dat" using ($4/100000):16 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.15.Cov25000.Y50000.dat" using ($4/100000):16 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.15.Cov100000.Y50000.dat" using ($4/100000):16 with lines lw 2 title "full"
set origin 0.54, .27
set ylabel "Alarm Fraction (e=0.25)" 1.3
plot[-.01:.51][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y50000.dat" using ($4/100000):16 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y50000.dat" using ($4/100000):16 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y50000.dat" using ($4/100000):16 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y50000.dat" using ($4/100000):16 with lines lw 2 title "full"


set origin 0.81, .27
set ylabel "False Positive Fraction (e=0.15)" 1.3
plot[-.01:.51][-.1:1.1] "munged.targeted.FM.0.15.Cov1562.Y50000.dat" using ($4/100000):12 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.15.Cov6250.Y50000.dat" using ($4/100000):12 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.15.Cov25000.Y50000.dat" using ($4/100000):12 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.15.Cov100000.Y50000.dat" using ($4/100000):12 with lines lw 2 title "full"

set origin 1.08, .27
set ylabel "False Positive Fraction (e=0.25)" 1.3
plot[-.01:.51][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y50000.dat" using ($4/100000):12 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y50000.dat" using ($4/100000):12 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y50000.dat" using ($4/100000):12 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y50000.dat" using ($4/100000):12 with lines lw 2 title "full"








set size 0.30, .30
set origin 0, 0
set xlabel "Target selectivity"
set xtics rotate by 90
set ylabel "" 1.3
plot [-.01:0.71][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y70000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y70000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y70000.dat" using ($4/100000):($10/100000) with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y70000.dat" using ($4/100000):($10/100000) with lines lw 2 title "full"


set origin 0.27, 0
set ylabel "" 1.3
plot[-.01:.71][-.1:1.1] "munged.targeted.FM.0.15.Cov1562.Y70000.dat" using ($4/100000):16 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.15.Cov6250.Y70000.dat" using ($4/100000):16 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.15.Cov25000.Y70000.dat" using ($4/100000):16 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.15.Cov100000.Y70000.dat" using ($4/100000):16 with lines lw 2 title "full"

set origin 0.54, 0
set ylabel "" 1.3
plot[-.01:.71][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y70000.dat" using ($4/100000):16 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y70000.dat" using ($4/100000):16 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y70000.dat" using ($4/100000):16 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y70000.dat" using ($4/100000):16 with lines lw 2 title "full"

set origin 0.81, 0
set ylabel "" 1.3
plot[-.01:.71][-.1:1.1] "munged.targeted.FM.0.15.Cov1562.Y70000.dat" using ($4/100000):12 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.15.Cov6250.Y70000.dat" using ($4/100000):12 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.15.Cov25000.Y70000.dat" using ($4/100000):12 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.15.Cov100000.Y70000.dat" using ($4/100000):12 with lines lw 2 title "full"

set origin 1.08, 0
set ylabel "" 1.3
plot[-.01:.71][-.1:1.1] "munged.targeted.FM.0.25.Cov1562.Y70000.dat" using ($4/100000):12 with lines lw 2 title "1/64",\
		"munged.targeted.FM.0.25.Cov6250.Y70000.dat" using ($4/100000):12 with lines lw 2 title "1/16",\
		"munged.targeted.FM.0.25.Cov25000.Y70000.dat" using ($4/100000):12 with lines lw 2 title "1/4",\
		"munged.targeted.FM.0.25.Cov100000.Y70000.dat" using ($4/100000):12 with lines lw 2 title "full"

set nomultiplot





