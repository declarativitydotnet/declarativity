function entropy=gEntropy(cov, given, base)

if ~exist('base', 'var')
  base=2;
end

if length(given)>0
  [a,A,S]=gJoint2Conditional(zeros(length(cov),1), cov, given);
else
  S=cov;
  a=cov;
end

entropy=log(sqrt( ((2*pi*exp(1))^length(a)) * det(S))) / log(base);
