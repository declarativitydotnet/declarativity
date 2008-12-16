function [y,p] = classifymog(x,s)
lik = lmog(x,s);
cp  = lik*s.cp'; % n x |Y|
[p,y]=max(cp,[],2);
y=y-1;
