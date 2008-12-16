function c=ncutcluster(s, minsize)

if ~exist('minsize','var')
  minsize = max(length(s) / 20,2);
end

D2inv = diag(sparse(1./sqrt(sum(s,1))));
[v,d]=eigs(D2inv*s*D2inv,2);
v = D2inv*v(:,2);

a = find(v>0);
b = find(v<=0);

if length(a) < minsize || length(b) < minsize
  c = ones(size(s,1),1);
else
  ca = ncutcluster(s(a,a), minsize);
  cb = ncutcluster(s(b,b), minsize);
  c(a) = ca;
  c(b) = cb+max(ca);
end
