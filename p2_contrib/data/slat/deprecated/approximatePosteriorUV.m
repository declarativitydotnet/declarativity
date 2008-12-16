function [mu, sigma, likelihood]=approximatePosteriorUV(muX, sigmaX, fn, Q, obs, param, warn_invalid)
%[mu, sigma, likelihood]=approximatePosteriorUV(muX, sigmaX, fn, Q, obs, param)

if ~exist('warn_invalid', 'var')
  warn_invalid=0;
end

%Try the two approximations and see which them returns higher likelihoood
%The strandard method first
[mu1, sigma1, muY, sigmaY, valid1] = approximateJoint(muX, sigmaX, fn, Q, param, warn_invalid);
if valid1
  l1 = mvnpdf(obs', muY', sigmaY);
else
  l1=0;
end

if 0 %l1<1e-8
%Now use the inverse camera location proposal
%Note that the UV is independent of the angle in this representation
qmu = zeros(7,1); % L, L0, U, V, Theta
qcov= zeros(7,7);
ii = [1 2 7];
A = [1 0 0; ...
     0 1 0; ...
     1 0 0; ...
     0 1 0; ...
     0 0 0; ...
     0 0 0; ...
     0 0 1];
qmu=A*muX(ii);
qcov=A*sigmaX(ii,ii)*A';
qcov(3:4,3:4)=qcov(3:4,3:4)+eye(2)*1e-4; %Add tiny amount of noise

param2.KK     = param.calib.KK;
param2.height = param.calib.pos(3);
param2.R      = panTilt2R(0, param.calib.tilt);
param2.P      = [0;0;1.8];
[qmu(5:6), qcov(5:6,5:6)] = approximateGaussian(obs, Q*4, @fnCameraCenter, zeros(2), param2);
% zeros(2) could also be eye(2)*0.01

[mu2, sigma2, l2] =...
  approximatePosteriorProposal(muX, sigmaX, qmu, qcov, @fnProjection, Q, obs, param, 0);
% Should also test this with chol, rather than sqrtm
else
  l2=0;
  l1
end

if l1>=l2 && valid1
  [b, B, S2] = gJoint2Conditional(mu1, sigma1, length(muX)+[1 2]);
  mu    = b+B*obs; 
  sigma = S2;
  likelihood=l1;
elseif l2>0
  disp('Using proposal distribution linearization')
  mu=mu2;
  sigma=sigma2;
  likelihood=l2;
else
  warning('Both likelihoods are zero! Have we diverged?')
  %[b, B, S2] = gJoint2Conditional(mu1, sigma1, length(muX)+[1 2]);
  %mu    = b+B*obs; 
  %sigma = S2;
  likelihood=l1;
  mu=muX;
  sigma=sigmaX;
end

      

if any(any(isnan(sigma)))
end

  
