function s=compute_stats(name, s)
import prl.*
load(name);
m = length(s.bel);
n = s.n;
tv = tempvars(n);
bv = biasvars(n);
temp = zeros(m,n)*nan;
bias = zeros(m,n)*nan;
for i=1:m
  for j=1:n
    if s.bel(i,j).arguments.contains(tv(j))
      try
        temp(i,j)=s.bel(i,j).marginal(domain(tv(j))).as_canonical_gaussian.as_moment_gaussian.mean;
      catch err
        err
      end
    end
    if s.bel(i,j).arguments.contains(bv(j))
      try
        bias(i,j)=s.bel(i,j).marginal(domain(bv(j))).as_canonical_gaussian.as_moment_gaussian.mean;
      catch err
        err
      end
    end
  end
end

s.temp=temp;
s.bias=bias;
s.rms=rms(model.bias(1:n)', bias);
