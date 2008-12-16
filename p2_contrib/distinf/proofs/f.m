function val=f(r,k,n)
val=(1+(n-1)*(r.^(k+1)))./(1+(n-1)*(r.^k));
i=r<0;
val(i)=nan;

