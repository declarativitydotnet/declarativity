function [mu, sigma] = conditionOnDiscreteSampling(muX, sigmaX, fn, fvalue, param)
% [mu, sigma] = conditionOnDiscrete(muX, sigmaX, fn, fvalue, param)
% Let A=fn(X) be a discrete random variable, which is a function of X.
% conditionOnDiscrete computes an approximation of P(X|A=fvalue)
% 
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.

% This function computes the approximation by random sampling

nSamples = 5000;
nCutoff = 1000;
d = length(muX);
A = sqrtm(sigmaX);
p = zeros(d,0);

while prod(size(p))<d*nCutoff
    p1 = A*randn(d, nSamples)+muX*ones(1,nSamples);
    f0 = (fn(p1, param)==fvalue);
    p = [p p1(:,find(f0))];
    display([num2str(prod(size(p))/d) ' samples accepted so far']);
end

mu=mean(p,2);
sigma=cov(p',1);

mineig = min(eig(sigma));
if mineig<=0
  warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig)]);
end
