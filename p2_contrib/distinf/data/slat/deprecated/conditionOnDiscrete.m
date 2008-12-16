function [mu, sigma] = conditionOnDiscrete(muX, sigmaX, fn, fvalue, param)
% [mu, sigma] = conditionOnDiscrete(muX, sigmaX, fn, fvalue, param)
% Let A=fn(X) be a discrete random variable, which is a function of X.
% conditionOnDiscrete computes an approximation of P(X|A=fvalue)
% 
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.

% This function implements precision 5 monomial method

d = length(muX);
u=sqrt(3);
w0 = 1+(d^2 - 7*d)/18;
w1 = (4-d)/18;
w2 = 1/36;

p0 = zeros(d,1);
p1 = u*[eye(d) -eye(d)];
p2 = zeros(d,0);

for i=1:d-1,
  %i is the index of the first non-zero coordinate
  pts = u*[zeros(i-1, 4*(d-i));
    ones(1,2*(d-i)) -ones(1,2*(d-i));
    eye(d-i) -eye(d-i) eye(d-i) -eye(d-i)];
  p2 = [p2 pts];
end
[stuff, np2] = size(p2);

A = sqrtm(sigmaX);
[stuff,np1] = size(p1);
[stuff,np2] = size(p2);
p0 = A*p0 + muX;
p1 = A*p1 + muX*ones(1,np1);
p2 = A*p2 + muX*ones(1,np2);

if exist('param', 'var')
  f0 = (fn(p0, param) == fvalue);
  f1 = (fn(p1, param) == fvalue);
  f2 = (fn(p2, param) == fvalue);
else
  f0 = (fn(p0) == fvalue);
  f1 = (fn(p1) == fvalue);
  f2 = (fn(p2) == fvalue);
end  

naccepted = sum(f0)+sum(f1)+sum(f2);

disp([num2str(naccepted) '/' num2str(1+d*2+np2) ' points accepted.']);

if naccepted==0
  mu = muX;
  sigma = sigmaX;
end

wt = w0*sum(f0)+w1*sum(f1)+w2*sum(f2);

mu = (w0*sum((ones(d,1)*f0).*p0,2) + ...
      w1*sum((ones(d,1)*f1).*p1,2) + ...
      w2*sum((ones(d,1)*f2).*p2,2)) / wt;

sigma = w0*p0*p0';
for i=1:np1,
  sigma = sigma + w1*f1(i)*p1(:,i)*p1(:,i)';
end
for i=1:np2,
  sigma = sigma + w2*f2(i)*p2(:,i)*p2(:,i)';
end
sigma = sigma/wt - mu*mu';

mineig = min(eig(sigma));
if mineig<=0
  warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig)]);
end
