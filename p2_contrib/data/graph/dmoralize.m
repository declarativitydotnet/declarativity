function b=dmoralize(a)

b = a;

assert(a==triu(a));

assert(size(a,1)==size(a,2));

for i=1:length(a)
  pa = find(a(:,i));
  b(pa,pa) = 1;
end

b = triu(b);
