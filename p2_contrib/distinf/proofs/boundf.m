function [val,r]=boundf(a,n)
% Evaluates the bound as a function of a

val = zeros(size(a));
r = zeros(size(a));

for i=1:numel(a)
  ai=a(i);
  r=(ai/(n-1))^(1/(ai+1))
  lam=(ai+1)*r;
  val(i)=f(r,ai,n)-lam*r;
end

