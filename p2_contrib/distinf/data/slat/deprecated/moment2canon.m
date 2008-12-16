function [K,h]=moment2canon(mu, sigma)
n=length(mu);
if any(size(mu)~=[n 1]) || any(size(sigma)~=[n n])
    error('AAA','mu and sigma do not have the right form');
end

K=inv(sigma);
h=K*mu;
%g=-0.5*mu'*K*mu-log((2*pi)^(n/2)*det(sigma)^0.5);
