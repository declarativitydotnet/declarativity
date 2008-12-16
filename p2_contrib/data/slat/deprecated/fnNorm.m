function fx = fnNorm(x, p)
% Computes the Lp norm of each column in x
% p=2 by default

if ~exist('p',var)
  p=2;
end

fx=sum(abs(x).^p,1).^(1/p);
