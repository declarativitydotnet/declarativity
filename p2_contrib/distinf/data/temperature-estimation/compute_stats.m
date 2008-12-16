function s=compute_stats(name, s)
import prl.*
load(name);
m = length(s.bel);
n = s.n;
tv = tempvars(n);
temp = zeros(m,n);
for i=1:m
  for j=1:n
    temp(i,j)=s.bel(i,j).marginal(domain(tv(j))).flatten.as_moment_gaussian.mean;
  end
end

s.temp=temp;
s.rms=rms(model.tmean(1:n)', temp);
