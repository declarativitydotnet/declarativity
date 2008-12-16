function [mu, sigma, muY, sigmaY, covXY] = approximateJoint2(muX, sigmaX, fn, Q, param)
% [mu, sigma, muY, sigmaY, covXY] = approximateJoint2(muX, sigmaX, fn, Q, param)
% Computes the joint of normal random vectors X and Y=fn(X)+N(0,Q)
% where fn is a deterministic function of X.  This version treats 
% last 2 columns of muX as angle and generates separate points for them.
%
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.

% This function implements precision 5 monomial method

nAngleSteps=100;

d = length(muX) -2;
u=sqrt(3);
w0 = 1+(d^2 - 7*d)/18;
w1 = (4-d)/18;
w2 = 1/36;

p=[];
peval=[];
w=[];

[a0, A, S]=gJoint2Conditional(muX, sigmaX, [d+1 d+2]);
sigmaX=(sigmaX+sigmaX')/2;

muT=muX([d+1 d+2])
sigmaT=sigmaX([d+1 d+2],[d+1 d+2]);
theta=(0:2*pi/nAngleSteps:(2*pi-1e-6))';
wt=mvnpdf([cos(theta) sin(theta)],muT',sigmaT);
wt=wt/sum(wt);
b0s=[];
for ti=0:(nAngleSteps-1),
  if wt(ti+1)>1e-10
  theta=2*pi*ti/nAngleSteps;
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

  B = sqrtm(S);
  b0= a0+A*[cos(theta);sin(theta)];
  b0s=[b0s b0];
  %A = chol(sigmaX);
  [stuff,np1] = size(p1);
  [stuff,np2] = size(p2);
  p0 = B*p0 + b0;
  p1 = B*p1 + b0*ones(1,np1);
  p2 = B*p2 + b0*ones(1,np2);
  p = [p [p0 p1 p2;...
          cos(theta)*ones(1, 1+np1+np2); ...
          sin(theta)*ones(1, 1+np1+np2)]];
  peval=[peval [p0 p1 p2; theta*ones(1, 1+np1+np2)]];
  w=[w wt(ti+1)*[w0 w1*ones(1, np1) w2*ones(1,np2)]];
  end
end

if exist('param', 'var')
  [f,valid] = fn(peval, param);
else
  [f,valid] = fn(peval);
end
if ~all(valid) 
  warning('Some integration points were outside of the domain of the function.')
end

ny=size(f,1);
nx=size(p,1);

if length(find(wt>1e-10))>1
  muX2= sum((ones(nx,1)*w).*p,2);
  sigmaX2= ((ones(nx,1)*w).*p)*(p')-muX2*muX2';
  sigmaX2=(sigmaX2+sigmaX2')/2;
else
  muX2=muX;
  sigmaX2=sigmaX;
end

muY = sum((ones(ny,1)*w).*f,2);
sigmaY=zeros(ny,ny);

for i=1:size(p,2),
  sigmaY = sigmaY + w(i)*f(:,i)*f(:,i)';
end

sigmaY = sigmaY - muY*muY';
sigmaY = sigmaY+Q;
sigmaY = (sigmaY+sigmaY')/2;

covXY = zeros(d+2,ny);
for i=1:size(p,2),
  covXY = covXY + w(i)*p(:,i)*f(:,i)';
end
covXY = covXY - muX*muY';

mu = [muX2; muY];
sigma = [sigmaX2 covXY; covXY' sigmaY];
mineig = min(eig(sigma));
if mineig<=0
  warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig) '; adding.']);
  sigma = sigma+(abs(mineig)+1e-8)*eye(length(mu));
end
