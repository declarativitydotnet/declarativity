param.A=[2 3; 0 1; 1 0]';
param.b=[0 1]';
mu=[1 0 -1]';
sigma = eye(3) + [0 0.1 0; 0.1 0 0; 0 0 0]

disp(['The following should be the same as the result of numeric integration'])
param.A*mu+param.b
param.A*sigma*param.A'
sigma*param.A'

[muY, sigmaY, covXY]=approximateJoint(mu, sigma, @fnLinear, ...
    zeros(2,2), param)

disp(['The following should be muY=1, sigmaY=3 (but is not due to' ...
      ' low degree of integration)'])

[muY, sigmaY, covXY]=approximateJoint(0,1, @sqr, 0)

