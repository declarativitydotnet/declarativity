function [mu, sigma] = approximatePosterior2(muX, sigmaX, fn, Q, obs, param)
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
n=length(muX); 
d=n-2;

% Compute the CPD of the camera and location parameters, given the angle
[a0, A, S]=gJoint2Conditional(muX, sigmaX, [d+1 d+2]);
sigmaX=(sigmaX+sigmaX')/2;

muT=muX([d+1 d+2])
sigmaT=sigmaX([d+1 d+2],[d+1 d+2]);
thetas=(0:2*pi/nAngleSteps:(2*pi-1e-6))';
wt=mvnpdf([cos(thetas) sin(thetas)],muT',sigmaT);
wt=wt/sum(wt);
p=[cos(thetas');sin(thetas')];

mus=zeros(n, nAngleSteps);
covs=zeros(n, n, nAngleSteps);

for i=1:nAngleSteps
    theta= thetas(i);
    muTi = theta;
    varTi= 1e-10;%(2*pi/nAngleSteps)^2;
    muXi = a0+A*[cos(theta);sin(theta)];
    covXi= S;
    mui  = [muXi; muTi];
    covi = [covXi zeros(length(muXi),1); zeros(1,length(muXi),1) varTi];
    % This covariance is only approximate
    
    % Now linearize the observation
    [mu, sigma, muY, sigmaY] = approximateJoint(mui, covi, ...
      @fnProjection, Q, param);
    
    % Update the weight
    wt(i) = wt(i)*mvnpdf(obs', muY', sigmaY);
    
    % Condition on the observation
    [b, B, S2] = gJoint2Conditional(mu, sigma, length(mui)+[1 2]);
    muXi=b+B*obs;
    mus(:,i)=[muXi(1:end-1); cos(theta); sin(theta)];
    covs(:,:,i)=[S2(1:end-1,1:end-1) zeros(d, 2); ...
      zeros(2, d) eye(2)*(2*pi/nAngleSteps)^2];
end

% Now project the mixture 
if sum(wt)>0
  wt=wt/sum(wt);
else
  % Ignore the likelihood, and just use the old weights
  warning('Zero weights -- have we diveged?')
  wt=mvnpdf([cos(thetas) sin(thetas)],muT',sigmaT);
  wt=wt/sum(wt);
end  
[mu, sigma]=mixture2gaussian(wt, mus, covs);

mineig = min(eig(sigma));
if mineig<=0
  warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig) '; adding.']);
  sigma = sigma+(abs(mineig)+1e-8)*eye(length(mu));
end
