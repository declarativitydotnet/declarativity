function [Ey,Ey2]=gquadrature(mu, sigma, fn);
% Computes the mean of the function, in the given normal
% distribution (mu, sigma)


A=sqrtm(sigma);

x = [-1 1];

Ey=0.5*sum(fn(A*x+mu));
Ey2=0.5*sum(fn(A*x+mu).*fn(A*x+mu));  % need to check this

