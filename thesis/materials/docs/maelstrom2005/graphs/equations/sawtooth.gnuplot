set term postscript eps monochrome dashed 13
set size 1, .4
set output "sawtooth.eps"

f=0.15
T=960
h=5
gamma1=0.031
gamma2=0.53
beta=0.1
nmin=16
aCT(x)=x
recip(x)=1/x
const(x,y)=1/(1-aCT(y))**(x)-1
poison(gamma,t,f,h)=1-1/(const(h,f)*exp(beta*gamma*(h)*t/nmin)+1)**(0.2)
periodic(gamma,f,t,T,h)=poison(gamma,int(t)%T,f,h)
success(gamma,f,t,T,h)=(1-periodic(gamma,f,t,T,h))**(h-1)
secs(d)=d*86400

set multiplot

set data style linespoints 
set ylabel "Poisoning" 1
set xlabel "Time"
set format x ""

set origin 0, 0
set size .38, .4
set nokey
set label 11 "No reset" at graph .5, .6 center
plot [t=0:4*T-10][0:1] periodic(gamma1,f,t,T*4,h) title 'Instantaneous',\
		       periodic(gamma1,f,T*2,T*4,h) title 'Average'

set origin .33, 0
set size .33, .4
set ylabel ""
set format y ""
set key
set label 11 "Reset once" at graph .5, .6 center
plot [t=0:4*T-10][0:1] periodic(gamma1,f,t,T*2,h) title 'Instantaneous',\
		       periodic(gamma1,f,T,T*2,h) title 'Average'

set origin .61, 0
set size .33, .4
set nokey
set label 11 "Reset twice" at graph .5, .6 center
plot [t=0:4*T-10][0:1] periodic(gamma1,f,t,T,h) title 'Instantaneous',\
		       periodic(gamma1,f,T/2,T,h) title 'Average'
		       


set nomultiplot
