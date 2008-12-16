function [r,val]=minfr(k,n)
% Evaluates the bound as a function of k

val = zeros(size(k));
r = zeros(size(k));

for i=1:numel(k)
  [r(i),val(i)]=fminsearch(@(r) f(r,k(i),n), 1);
end

