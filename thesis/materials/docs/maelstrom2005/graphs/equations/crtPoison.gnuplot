set term postscript eps monochrome dashed  13
set output "poisoning-CRT.eps"

set size .7, .6
secs(d)=d*86400
set xlabel 'days'
set xtics ("1" secs(1), "2" secs(2), "3" secs(3), "4" secs(4), "5" secs(5), "6" secs(6), "7" secs(7), "8" secs(8), "9" secs(9), "10" secs(10))
set data style lines
set key outside
plot [][0:1.01] "CRT5" title 'f=5%',\
	"CRT15" title 'f=15%',\
	"CRT25" title 'f=25%',\
	"CRT26.15" title 'f=26.15%',\
	"CRT26.155" title 'f=26.155%',\
	"CRT26.16" title 'f=26.16%',\
	"CRT26.2" title 'f=26.2%',\
	"CRT27" title 'f=27%',\
	"CRT30" title 'f=30%'


