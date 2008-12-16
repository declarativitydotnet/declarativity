function [x2, P2, valid]=slatToCameraXy(x, P, model, transform, cams)

%converts the distribution xt, Pt to a representation of 
%Abstract intepretation: you pass a factor to the function that 
%represents the marginal over variables that include l0, u, v;
%The function computes an approximation to the distributino over
%the additional variables (x,y) and marginalizes out l0, u, v.
%In our case, it just overwrites x,y in xt, Pt.
%due to the limitaions of our current implementation, the function
%does not return the correct correlations between xy and the rest
%of the model.

%in the future, we need to keep the variable handles somewhere, 
%(or have a way to look them by having standardized names)
%the rest will follow easily

if ~exist('transform', 'var')
  c = 1;
  R = eye(2);
  t = zeros(2,1);
else
  c = transform.c;
  R = transform.R;
  t = transform.t;
end

if ~exist('cams','var')
  cams = 1:model.nCams;
end

if numel(P)==0, 
  P = eye(size(x,1))*1e-60; % hack hack hack
end

x2 = x;
P2 = P;
% R(1,1)=cos(dtheta), R(2,1)=-sin(dtheta) in the standard counter-clockwise
% angle notation. Our cameras pan clockwise.
dT = atan2(R(1,2),R(1,1));

valid=ones(length(cams),1);

% update the person's position
x2(1:2)=c*R*x2(1:2)+t;
P2(1:2,1:2)=c^2*R*P2(1:2,1:2)*R';

for k=1:length(cams)
  ci = [1 2 model.ci(k,:)];
  ci1= [model.ci(k,1:2)];
  muX=x(ci);
  sigmaX=full(P(ci,ci));
  if model.param(k).repr==2,
    [muX,sigmaX] = approximateGaussian(muX, sigmaX,...
      @fnUvCameraCenter, eye(2)*1e-6, model.param(cams(k)));
  else
    muX=muX(3:4);
    sigmaX=sigmaX(3:4,3:4);
  end
  muX=c*R*muX+t;
  sigmaX=c^2*R*sigmaX*R';
  x2(ci1)=muX;
  P2(ci1,ci1)=sigmaX;
  pani=ci(model.param(k).pani);
  x2(pani)=x2(pani)+dT;
  if sigmaX(1,1)>9000
    valid(k)=0;
  end
  
end  
  