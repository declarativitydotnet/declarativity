function [v,b] = pca(a,k,normcolumns)

if nargin>2 && normcolumns
  b=normcols(a);
else
  b=a./repmat(sum(a,2),1,size(a,2));
end
c=cov(b);
[v,d]=eigs(c,k);
v=v*diag(sign(v(1,:))); % normalize
