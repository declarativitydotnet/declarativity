function [val,r]=minfn(k,n)
% Evaluates the bound as a function of n

val = zeros(size(n));
r = zeros(size(n));

for i=1:numel(n)
  [r(i),val(i)]=fminsearch(@(r) f(r,k,n(i)), 1);
end

