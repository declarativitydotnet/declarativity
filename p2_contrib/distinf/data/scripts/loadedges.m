function [adj,t]=loadedges(filename)
a=load(filename);
assert(size(a,2)==4);
[y,i]=sort(a(:,end));
a=a(i,:);
n = max(max(a(:,1:2)));
A = sparse(n,n);
k=1;
for i=1:size(a,1)
  A(a(i,1),a(i,2))=a(i,3);
  if i==size(a,1) || a(i,end)~=a(i+1,end)
    adj{k} = A;
    t(k,1) = a(i,end);
    k = k+1;
  end
end

