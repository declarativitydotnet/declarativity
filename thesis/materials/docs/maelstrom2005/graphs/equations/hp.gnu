set term postscript eps monochrome dashed 13
set output "successful-routing.eps"
set size 1.4, .6
set data style lines

set multiplot
set origin 0.0,0.0 
set size .7, .6
set grid
set ytic (0.2,0.4,0.5,0.6,0.8,1.0)
set ylabel "Prob. of successful routing"
set xlabel "Routing table poisoning" 
set title "(a) Successful routing vs. poisoning for different hopcounts "
#set key 30,6 spacing 2 
plot [0:1] [0:1] (1-x)**2 title 'Hopcount=2',\
          (1-x)**3 title 'Hopcount=3',\
          (1-x)**4 title 'Hopcount=4',\
          (1-x)**5 title 'Hopcount=5'
          

 
#set output "successful-redundancy.eps"
set origin 0.7,0.0 
set size 0.7, .6
set noytic
set grid
set ylabel
#set ylabel "Prob. of successful routing"
set xlabel "Routing table poisoning" 
set title "(b) Successful routing vs. poisoning (Hopcount=4)"
plot [0:1] [0:1] (1-(1-(1-x)**4)**4) title 'Redundancy=4',\
          (1-(1-(1-x)**4)**8) title 'Redundancy=8',\
          (1-(1-(1-x)**4)**16) title 'Redundancy=16',\
          (1-(1-(1-x)**4)**32) title 'Redundancy=32',\
          (1-(1-(1-x)**4)**64) title 'Redundancy=64'




#set output "successful-sybil.eps"
#set origin 0.0,0.0 
#set size 1.0, 1.0
#set grid 
#set ylabel "Prob. of successful routing"
#set xlabel "Malicious frac in RT" 
#set title "Total population = 100,000 (# Max Hops = 10)"
#plot [0:1] [0:1] (1-(1-(1-x)**10)**4) title 'Redundancy=4' w lp lt 1,\
#          (1-(1-(1-x)**10)**8) title 'Redundancy=8' w lp lt 2,\
#          (1-(1-(1-x)**10)**16) title 'Redundancy=16' w lp lt 3,\
#          (1-(1-(1-x)**10)**32) title 'Redundancy=32' w lp lt 4,\
#          (1-(1-(1-x)**10)**64) title 'Redundancy=64' w lp lt 5




