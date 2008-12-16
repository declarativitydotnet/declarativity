function [mu, sigma, likelihood] = ...
  approximatePosteriorProposal(muX, sigmaX, qmuX, qsigmaX, fn, Q, obs, param, warn_invalid)
%[mu, sigma, likelihood] = ...
%  approximatePosteriorProposal(muX, sigmaX, qmuX, qsigmaX, fn, Q, obs, param)
% Computes the joint of normal random vectors X and Y=fn(X)+N(0,Q)
% where fn is a deterministic function of X.
% 
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.


% % This function implements precision 5 monomial method
% 
d = length(muX);
% u=sqrt(3);
% w0 = 1+(d^2 - 7*d)/18;
% w1 = (4-d)/18;
% w2 = 1/36;
% 
% p0 = zeros(d,1);
% p1 = u*[eye(d) -eye(d)];
% pts = [p0 p1];
% wts = [w0 w1*ones(1,2*d)];
% 
% for i=1:d-1,
%   %i is the index of the first non-zero coordinate
%   pt = u*[zeros(i-1, 4*(d-i));
%     ones(1,2*(d-i)) -ones(1,2*(d-i));
%     eye(d-i) -eye(d-i) eye(d-i) -eye(d-i)];
%   pts = [pts pt];
%   wts = [wts w2*ones(1,size(pt,2))];
% end
% np = size(pts, 2);

%resolution=3;
[p1,p2,p3,p4,p5,p6,p7]=ndgrid(-1:1);
%pts=[p1(:) p2(:) p3(:) p4(:) p5(:) p6(:) p7(:)]';
[p1,p2,p3,p4,p5]=ndgrid(-1:1);
pts=[p1(:) p2(:) p3(:) p4(:) p5(:)]';
np=size(pts,2);

% Linearize around a proposal
A = sqrtm(qsigmaX);
%A = chol(sigmaX);
pts = A*pts + qmuX*ones(1,np);

wts=mvnpdf(pts', qmuX', qsigmaX)';
wts=wts/sum(wts);

if exist('param', 'var')
  [fx,valid] = fn(pts, param);
else
  [fx,valid] = fn(pts);
end  

if any(~valid)
  if (~exist('warn_invalid', 'var') || warn_invalid==1)
    warning('Some of the integration points are invalid')
  end
end

% reweight

sigmaX=(sigmaX+sigmaX')/2;
qsigmaX=(qsigmaX+qsigmaX')/2;
wts = wts.*(mvnpdf(fx', obs', Q).*mvnpdf(pts', muX', sigmaX)./mvnpdf(pts', qmuX', qsigmaX))';
likelihood = sum(wts);
if any(~valid)
  likelihood=0;
end
if likelihood==0,
  if (~exist('warn_invalid', 'var') || warn_invalid==1)
    warning('0 likelihood')
  end
  mu=muX;
  sigma=sigmaX;
  return; %there's nothing we can do
end  

%Now compute E[X|y] and Cov(X|y)
ptswt = pts.*(ones(d,1)*wts);
mu = sum(ptswt,2)/likelihood;
sigma = ptswt*pts'/likelihood - mu*mu';
sigma = (sigma+sigma')/2;

mineig = min(eig(sigma));
if mineig<=0
  disp(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig) '; adding.']);
  sigma = sigma+(abs(mineig)*2)*eye(length(mu));
end

mineig=min(eig(inv(sigma)-inv(sigmaX)));
if mineig<=0
  disp(['Warning: Posterior less certain than prior; mineig of precision' num2str(mineig)])
end
