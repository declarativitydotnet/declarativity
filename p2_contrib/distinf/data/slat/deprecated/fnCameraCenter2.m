function [cpos,valid] = fnCameraCenter2(x, param)
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

xx  = inv(param.KK)*[x(1:2,:);ones(1,nx)];

A = [-1 0 nan; 0 -1 nan; -param.R(:,3)'];

for i=1:nx,
  rP = param.R*[x(3:4,i); 1.8];
  A(1:2,3)=xx(1:2,i);
  b=[rP(1:2)-rP(3)*xx(1:2,i); param.height];
  t=A\b;
  cpos(:,i)=-param.R'*t;
end

cpos=cpos(1:2,:);
valid=ones(1,nx);
