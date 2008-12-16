function [mu, sigma, muY, sigmaY, valid] = approximateJoint(muX, sigmaX, fn, Q, param, warn_invalid)
% [mu, sigma, muY, sigmaY, covXY] = approximateJoint(muX, sigmaX, fn, Q, param, qmuX, qsigmaX)
% Computes the joint of normal random vectors X and Y=fn(X)+N(0,Q)
% where fn is a deterministic function of X.
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
pts = [p0 p1];
wts = [w0 w1*ones(1,2*d)];

for i=1:d-1,
  %i is the index of the first non-zero coordinate
  pt = u*[zeros(i-1, 4*(d-i));
    ones(1,2*(d-i)) -ones(1,2*(d-i));
    eye(d-i) -eye(d-i) eye(d-i) -eye(d-i)];
  pts = [pts pt];
  wts = [wts w2*ones(1,size(pt,2))];
end
np = size(pts, 2);

% if exist('qmuX', 'var')
%   % Linearize around a proposal
%   A = sqrtm(qsigmaX);
%   %A = chol(sigmaX);
%   pts = A*pts + qmuX*ones(1,np);
%   % reweight
%   wts = wts.*(mvnpdf(pts', muX', sigmaX)./mvnpdf(pts', qmuX', qsigmaX))';
% else
  % Standard linearization
  %A = sqrtm(sigmaX);
  A = chol(sigmaX)';
  pts = A*pts + muX*ones(1,np);
% end

if exist('param', 'var')
  [fx,validpts] = fn(pts, param);
else
  [fx,validpts] = fn(pts);
end  

valid=all(validpts);
if any(~validpts) & (~exist('warn_invalid', 'var') || warn_invalid==1)
  warning('Some of the integration points are invalid, ignoring...')
end

if any(any(isinf(fx))) || any(any(isnan(fx)))
  warning('Some of the integration values are inf or nan.');
  valid=0;
end

if valid
  fxwt = fx.*(ones(size(fx,1),1)*wts);
  muY = sum(fxwt,2);
  sigmaY = fxwt*fx'-muY*muY'+Q;
  sigmaY = (sigmaY+sigmaY')/2;
  covXY  = pts*fxwt'-muX*muY';
 
  mu = [muX; muY];
  sigma = [sigmaX covXY; covXY' sigmaY];
  %save test
  mineig = min(eig(sigma));
  if mineig<=0
    warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
          num2str(mineig) '; adding.']);
    sigma = sigma+(abs(mineig)+1e-8)*eye(length(mu));
  end
  sigmaY=sigma(d+1:end,d+1:end);
else
  %save test
  muY = zeros(size(fx,1),1);
  sigmaY=eye(size(fx,1))*1e8;
  covXY=zeros(d,size(fx,1));
  mu = [muX; muY];
  sigma = [sigmaX covXY; covXY' sigmaY];
  sigma=(sigma+sigma')/2;
end
sigma=real((sigma+sigma')/2);
mu=real(mu);
