function [a0, A, S] = gJoint2Conditional(mu, sigma, ci)
%[a0, A, S] = gJoint2Conditional(mu, sigma, ci)
% Suppose that the set consists of two sets of variables, X & Y
% ci denotes the indices for the variables X
% returns Y|X

n = length(mu);
di =  setdiff(1:n, ci);

sigmaYX   =sigma(di,ci);
invsigmaXX=inv(sigma(ci,ci));
sigmaYY   =sigma(di,di);

a0 = mu(di)-sigmaYX*invsigmaXX*mu(ci);
A  = sigmaYX*invsigmaXX;
S  = sigmaYY-A*sigma(ci,di);
S  = (S+S')/2;

