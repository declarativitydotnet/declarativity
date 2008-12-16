function mog = approximateAngleDistribution(muX, sigmaX, fn, Q, param)
% [mu, sigma, muY, sigmaY, covXY] = approximateJoint2(muX, sigmaX, fn, Q, param)
% Computes the joint of normal random vectors X and Y=fn(X)+N(0,Q)
% where fn is a deterministic function of X.  This version treats 
% last 2 columns of muX as angle and generates separate points for them.
%
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.


nAngleSteps=20;%100;
maxStandardIntegration=0.15; % maximum standard deviation for detailed linz'n
ptSpread=2;
n=length(muX); 
d=n-1;

%Select the method for numerical integration
muT=muX(d+1);
sigmaT=sqrt(sigmaX(d+1, d+1));
if param.repr==1,
  mog.p=1;
  mog.mus=muX(3:4);
  mog.covs=sigmaX(3:4,3:4);
  return
elseif sigmaT<maxStandardIntegration %Standard integration
  mog.p=1;
  [mog.mus, mog.covs]=approximateGaussian(muX, sigmaX, fn, Q, param);
  return;
elseif sigmaT>pi/ptSpread
  thetas=(muT-pi:2*pi/nAngleSteps:(muT+pi-1e-6))';
  wt=normpdf(thetas, muT, sigmaT)+normpdf(thetas-2*pi,muT,sigmaT)+...
    normpdf(thetas+2*pi,muT,sigmaT);
else
  thetas=(muT-ptSpread*sigmaT:2*ptSpread*sigmaT/nAngleSteps:muT+ptSpread*sigmaT-1e-6)';
  %length(thetas)
  wt=normpdf(thetas, muT, sigmaT);
end
dtheta=thetas(2)-thetas(1);
wt=wt/sum(wt);

% Compute the CPD of the camera and location parameters, given the angle
[a0, A, S]=gJoint2Conditional(muX, sigmaX, d+1);
sigmaX=(sigmaX+sigmaX')/2;

mus=zeros(2, nAngleSteps);
covs=zeros(2, 2, nAngleSteps);

for i=1:nAngleSteps
  if wt(i)>1e-10 %verify this
    theta= thetas(i);
    muTi = theta;
    varTi= dtheta^2;  %1e-10;
    muXi = a0+A*theta;
    covXi= A*varTi*A'+S;
    covXT= A*varTi;
    mui  = [muXi; muTi];
    covi = [covXi covXT; covXT' varTi];
    
    % Now linearize the observation
    [mus(:,i), covs(:,:,i)] = approximateGaussian(mui, covi, ...
      fn, Q, param);
  else
    wt(i)=0;
  end
end

mog=struct('p', wt, 'mus', mus, 'covs', covs);

