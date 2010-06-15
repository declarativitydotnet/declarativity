set term postscript eps color dashed 24
set size 3.0, 1.0
set output "poisoning-T.eps"
set multiplot
f=0.25
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

set data style linespoints 
set ylabel "Routing table poisoning"
set xlabel "Time(Sec)"


set origin 0.0,0.0
set size 1.0,1.0
plot [t=0:4*T][0:1.01] periodic(gamma1,f,t,T/8,h) title 'Epoch length=120 sec' lw 4
set origin 1.0,0.0
plot [t=0:4*T][0:1.01] periodic(gamma1,f,t,T,h) title 'Epoch length=960 sec' lw 4
set origin 2.0,0.0
plot [t=0:4*T][0:1.01] periodic(gamma1,f,t,4*T,h) title 'Epoch length=3840 sec' lw 4
