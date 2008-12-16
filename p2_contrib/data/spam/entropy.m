function Y=entropy(X,dim)
Y=-X.*log(X);
i=find(X==0);
Y(i)=0;
Y=sum(Y,dim);

