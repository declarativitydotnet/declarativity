function [xmt,mut,sigmat] = slatMF1(problem, k, nPans)
% This function uses mixture of Gaussians to do the filtering.
% The cliques for the burning-ing cameras contain that camera's
% center position and the person location only, and the person
% location is marginalized at each time step.

if ~exist('nPans','var')
  nPans=90;
end

ts=find(problem.visible(k,:));
P=(0.10^2)*eye(2);
R = diag([problem.sigma_u problem.sigma_v] ./ ...
    problem.calib(k).imageSize).^2;

% Compute the initial distribution (based on the first observation)
x.pan = -pi:2*pi/nPans:pi-1e-6;
for i=1:length(x.pan),
  param(i).KK    =problem.calib(k).KK;
  param(i).height=problem.calib(k).pos(3);
  param(i).R     =panTilt2R(x.pan(i), problem.calib(k).tilt);
  param(i).P     =problem.pos(1:3,ts(1));
end
x.p = ones(1,nPans)/nPans;
[x.mu,x.sigma] = approximateGaussian(problem.obs(:,ts(1),k), ...
    eye(2)*9, @fnCameraCenter, P, param);
[mut(:,1) sigmat(:,:,1)]=mixture2gaussian(x.p,x.mu,x.sigma,x.pan);

% Compute the camera parameters for the observation likelihood
for i=1:length(x.pan),
  oparam(i).repr = 1;
  oparam(i).cposi = [3 4 0];
  oparam(i).pani  = 0;
  oparam(i).tilti = 0;
  oparam(i).calib = problem.calib(k);
  oparam(i).calib.pan = x.pan(i);
  oparam(i).trim = 1;
end

% Initialize logs
xmt(1:length(ts))=x;

% Update the distribution in each step
for t1=2:length(ts),
  t=ts(t1)
  obs = problem.obs(:,t,k)./(problem.calib(k).imageSize');
  pos = problem.pos(1:2,t);
  P=(0.10^2)*eye(2);
  %is=find(state>1e-5);
  %state(find(state<1e-20))=0;

  for i=1:length(x.pan),
    if x.p(i)>0
      [mu, sigma, muY, sigmaY] = approximateJoint([pos;x.mu(:,i)], ...
          [P zeros(2); zeros(2) x.sigma(:,:,i)], @fnProjection, R, oparam(i));
      x.p(i)=x.p(i)*mvnpdf(obs', muY', sigmaY);
      [a, A, S]=gJoint2Conditional(mu,sigma,[5 6]);
      mu1=a+A*obs;
      x.mu(:,i)=mu1(3:4);
      x.sigma(:,:,i)=S(3:4,3:4);
    end
  end
  x.p=x.p/sum(x.p);

  xmt(t1)=x;
  [mut(:,t1) sigmat(:,:,t1)]=mixture2gaussian(x.p,x.mu,x.sigma,x.pan);
end

  
