function [mu, sigma] = approximatePosterior(muX, sigmaX, fn, Q, obs, param, push_in)
% [mu, sigma, muY, sigmaY, covXY] = approximateJoint2(muX, sigmaX, fn, Q, param)
% Computes the joint of normal random vectors X and Y=fn(X)+N(0,Q)
% where fn is a deterministic function of X.  This version treats 
% last 2 columns of muX as angle and generates separate points for them.
%
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.

% Push the observations in?
if ~exist('push_in', 'var')
  push_in=2;
end

%push_in

nAngleSteps=50;
maxStandardIntegration=0.15; %0.15; % maximum standard deviation for detailed linz'n
ptSpread=3;
n=length(muX); 
d=n-1;

%Select the method for numerical integration
muT=muX(d+1);
sigmaT=sqrt(sigmaX(d+1, d+1));
if sigmaT<maxStandardIntegration %Standard integration
  [mu,sigma] = approximatePosteriorUV(muX, sigmaX, fn, Q, obs, param, 1);
  return;
elseif sigmaT>pi/ptSpread
  thetas=(muT-pi:2*pi/nAngleSteps:(muT+pi-1e-6))';
  wt=normpdf(thetas, muT, sigmaT)+normpdf(thetas-2*pi,muT,sigmaT)+...
    normpdf(thetas+2*pi,muT,sigmaT);
else
  thetas=(muT-ptSpread*sigmaT:2*ptSpread*sigmaT/nAngleSteps:muT+ptSpread*sigmaT-1e-6)';
  length(thetas)
  wt=normpdf(thetas, muT, sigmaT);
end
disp(['stdev=' num2str(sigmaT)]);
dtheta=thetas(2)-thetas(1);
wt=wt/sum(wt);
oldwt=wt;

[mucoeff,sigmacoeff]=mixture2gaussian(wt', thetas', ones(1,1,length(thetas))*1e-8);
multCoeff = sigmaT/sqrt(sigmacoeff);
disp(['Preservation coefficient: ' num2str(multCoeff)])

% Compute the CPD of the camera and location parameters, given the angle
[a0, A, S]=gJoint2Conditional(muX, sigmaX, d+1);
sigmaX=(sigmaX+sigmaX')/2;

if push_in==2,
mus=zeros(n, nAngleSteps);
covs=zeros(n, n, nAngleSteps);
else
mus=zeros(n+2, nAngleSteps);
covs=zeros(n+2, n+2, nAngleSteps);
end
maxlikelihood=0;

for i=1:nAngleSteps
  if wt(i)>1e-10 %verify this
    theta= thetas(i);
    muTi = theta;
    if param.repr==1,
      varTi = 1e-10;
    else
      varTi= 1e-8; %dtheta^2;  %4*dtheta^2;
    end
    muXi = a0+A*theta;
    covXi= A*varTi*A'+S;
    covXT= A*varTi;
    mui  = [muXi; muTi];
    covi = [covXi covXT; covXT' varTi];
    
    % Now linearize the observation
    if param.repr==1 && push_in==2
      qmu=mui(1:2);
      qcov=covi(1:2,1:2);
      param2=param;
      param2.KK= param.calib.KK;
      param2.height = param.calib.pos(3);
      param2.R = panTilt2R(theta, param.calib.tilt);
      param2.P = [0;0;1.8];
      [cmu,ccov]=approximateGaussian(obs, Q, @fnCameraCenter, zeros(2), param2);
      qmu=[qmu;cmu+qmu;muTi];
      qcov=[qcov qcov zeros(2,1); qcov qcov+ccov zeros(2,1); ...
            zeros(1,4) varTi];
          
      [mus(:,i), covs(:,:,i), likelihood]= ...
        approximatePosteriorProposal(mui, covi, qmu, qcov, @fnProjection, Q, obs, param);
      %was: approximatePosteriorPF
      wt(i)=wt(i)*likelihood;
    else
      %this code happens to work for the KF case, too 
%      [mu,sigma,likelihood] = approximatePosteriorUV(mui, covi, fn, Q, obs, param);
      [mu, sigma, muY, sigmaY] = approximateJoint(mui, covi, ...
        @fnProjection, Q, param);
      
      if push_in==2,
        likelihood = mvnpdf(obs', muY', sigmaY);
     
        % Condition on the observation
        [b, B, S2] = gJoint2Conditional(mu, sigma, length(mui)+[1 2]);
        mu=b+B*obs; 
        sigma=S2;
      else
        likelihood=1;
      end
      % Update the weight
      wt(i) = wt(i)*likelihood;
      mus(:,i) = mu; 
       covs(:,:,i)=sigma;
      maxlikelihood=max(maxlikelihood,likelihood);
    end
  else
    wt(i)=0;
  end
end
%maxlikelihood

% Now project the mixture 
if sum(wt)>0
  wt=wt/sum(wt);
else
  % Ignore the likelihood, and just use the old weights
  warning('Zero weights -- have we diverged?')
  wt=normpdf(thetas, muT, sigmaT)+normpdf(thetas-2*pi,muT,sigmaT)+...
  normpdf(thetas+2*pi,muT,sigmaT);
  wt=wt/sum(wt);
end 

if param.repr==1 && sigmaT>pi/ptSpread 
  % Compute the optimal shift
  mus2=mus(end,:);
  covs2=covs(end,end,:);
  [mu, bestvar]=mixture2gaussian(wt, mus2, covs2);
  besti=0;
  for i=1:nAngleSteps-1,
    mus2(i)=mus2(i)+2*pi;
    if wt(i)>0
      [mu, sigma]=mixture2gaussian(wt, mus2, covs2);
      if sigma<bestvar,
        besti=i;
        bestvar=sigma;
      end
    end
  end
  if besti>nAngleSteps/2
    mus(end,besti+1:end)=mus(end,besti+1:end)-2*pi;
  else
    mus(end,1:besti)=mus(end,1:besti)+2*pi;
  end
  disp(['Optimal range: ' num2str(mus(end,besti+1)) '; ' ...
  num2str(mus(end,mod(besti-1,nAngleSteps)+1))])
end

[mu, sigma]=mixture2gaussian(wt, mus, covs);
if push_in==1
  save test
   % Condition on the observation
   [b, B, S2] = gJoint2Conditional(mu, sigma, length(muX)+[1 2]);
   mu=b+B*obs; 
   sigma=S2;
end  
%A = eye(size(sigma,1));
%A(end,end)=multCoeff;
%sigma=A*sigma*A';

mineig = min(eig(sigma));
if mineig<=0
  warning(['approximateJoint: The resulting matrix is not P.D. Mineig=' ...
        num2str(mineig) '; adding.']);
  sigma = sigma+(abs(mineig)*2)*eye(length(mu));
end

lambdaln=inv(sigma)-inv(sigmaX);
e=eig(lambdaln);
mine=min(e);
maxe=max(e);
[log10(abs(mine)) log10(abs(maxe))]
if mine<0 & abs(maxe)/abs(mine)<10^14
  disp('The likelihood is not positive definite to working precision, adjusting')
  [v,d]=eig(lambdaln);
  i=find(diag(d)<0);
  d(i,i)=0;
  lambdaln2=v*d*v';
  sigma=inv(inv(sigmaX)+lambdaln2);
end
