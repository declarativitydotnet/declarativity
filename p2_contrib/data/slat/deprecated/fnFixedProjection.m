function [fx, valid] = fnFixedProjection(x, param)
% fx = fnFixedProjection(x, param)
%
% x is assumed to be a point in 2D plane at 1.8m
% param specifies the mapping of the parameters to vector x
%   and any constants that are used in computation.
% 
% param.calib - camera calibration
% param.xi    - point position indices([x y z], can be 0)
% param.x     - 3x1 array, containing the fixed parameters 

[stuff, nx] = size(x);

p=ones(4, nx);

calib=param.calib;

% Copy the points
for i=1:3,
  if param.xi(i) 
    p(i,:)=x(param.xi(i),:);
  else
    p(i,:)=param.x(i);
  end
end

% Compute the transform
fx = (calib.KK*[calib.R calib.T])*p;

% Divide by the 3rd (homogenous) coordinate and normalize
% by the image size, in order to keep the covariance matrices well-conditioned
fx = [fx(1,:)./fx(3,:)/param.calib.imageSize(1); ...
      fx(2,:)./fx(3,:)/param.calib.imageSize(2)];

% Check which points are valid
valid = (calib.R(3,:)*(p(1:3,:)-calib.pos*ones(1,nx)) > 0.01);
