function [mu, sigma, p, w] = visibilitySampling(muX, sigmaX, param, obs, R)
% [mu, sigma] = conditionOnDiscrete(muX, sigmaX, fn, fvalue, param)
% Let A=fn(X) be a discrete random variable, which is a function of X.
% conditionOnDiscrete computes an approximation of P(X|A=fvalue)
% 
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.

% This function computes the approximation by random sampling

nSamples = 5000;
nCutoff = 100;
d = length(muX);
A = sqrtm(sigmaX);
p = zeros(d,0);
accepted = 0;
w = zeros(0,0);
total=0;

while accepted<nCutoff
    p1 = A*randn(d, nSamples)+muX*ones(1,nSamples);
%    fv = fnVisibility(p1, param);
    fx = fnProjection(p1, param);
    w1 = mvnpdf(fx', obs', R)';
    i = find(w1>100);
    
    p = [p p1(:,i)];
    w = [w w1(i)];
    accepted = length(w);
    display([num2str(accepted) ' samples accepted so far']);
    total=total+nSamples;
end
disp(['Sample recall: ' num2str(accepted/total)]);

mu=sum((ones(d,1)*w).*p,2)/sum(w);
sigma=zeros(d);
for i=1:length(w),
  sigma=sigma+w(i)*p(:,i)*(p(:,i)');
end
sigma=sigma/sum(w)-mu*mu';
sigma=0.5*(sigma+sigma'); % to deal with round-off errors

mineig = min(eig(sigma));
if mineig<=0
  warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig)]);
end
