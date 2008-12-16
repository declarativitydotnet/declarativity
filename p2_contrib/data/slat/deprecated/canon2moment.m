function [mu, sigma]=canon2moment(K,h,g)
n=length(h);
if any(size(h)~=[n 1]) || any(size(K)~=[n n])
    error('AAA','mu and sigma do not have the right form');
end

sigma=inv(K);
mu=sigma*h;

if any(any(isnan(sigma)))
  warning('some of the entries in are NAN');
end
