function [v,b] = pca(a,k,normalize)

if normalize
  s=std(a,0,1);
  s(s==0)=1;
  b=a./repmat(s,size(a,1),1);
else
  b=a./repmat(sum(a,2),1,size(a,2));
end

c=cov(b);
[v,d]=eigs(c,k);
v=v*diag(sign(v(1,:))); % normalize

if normalize
  v=v./repmat(s',1,size(v,2));
  b=a;
end

