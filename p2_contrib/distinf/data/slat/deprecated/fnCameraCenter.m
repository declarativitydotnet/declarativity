function [cpos,valid] = fnCameraCenter(x, param)
% cpos = fnCameraCenter(x, param)
% x: 2xn - n observations
%
% param specifies the known camera parameters:
% 
% param.P  - point that's being projected
% param.KK - intrinsic calibration
% param.R  - rotation matrix
% param.height - height

[stuff, nx] = size(x);
cpos=zeros(3,nx);

x  = inv(param.KK)*[x;ones(1,nx)];
rP = param.R*param.P;

A = [-1 0 nan; 0 -1 nan; -param.R(:,3)'];

for i=1:nx,
  A(1:2,3)=x(1:2,nx);
  b=[rP(1:2)-rP(3)*x(1:2,nx); param.height];
  t=A\b;
  cpos(:,i)=-param.R'*t;
end

cpos=cpos(1:2,:);
valid=ones(1,nx);
