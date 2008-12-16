function corr=cov2correl(cov)
[n,n,m]=size(cov);

for i=1:m,
  a=sqrt(diag(cov(:,:,i)));
  corr(:,:,i)=cov(:,:,i)./(a*a');
end
