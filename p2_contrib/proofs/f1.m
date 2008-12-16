function val=f(r,k,n)
val=1./(1+(n-1)*(r.^k))+r;
i=r<0;
val(i)=nan;

