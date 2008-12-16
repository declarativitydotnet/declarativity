function [fx,valid] = fnProjection(x, param)
% fx = fnProjection(x, param)
%
% param specifies the mapping of the parameters to vector x
%   and any constants that are used in computation.
% 
% param.calib - camera calibration (only used for constants)
% param.repr  - representation of the parameters (1=>position + panTilt)
% param.cposi - camera position indices ([x y z], can be 0)
% param.pani  - camera pan index (0 => use pan from calib)
% param.tilti - camera tilt index (0 => use tilt from calib)

% standard height: 1.8m

[stuff, nx] = size(x);
valid=zeros(1,nx);

%if ~isfield(param, 'height')
%  param.height = 1.8;
%end

ppos = [x(1:2,:) ; param.height*ones(1,nx) ; ones(1,nx)];
[cpos, RR, TT] = decodeCameraState(x, param);

% Compute the transformed points
for i=1:nx,
  R = RR(:,:,i);
  T = TT(:,i);
  
  % check if the point is on the front side of the camera
  dir = R(3,:)*(ppos(1:3,i)-cpos(:,i));
  if dir >= 0.01 & ~isinf(dir) % we're okay
    fx(:,i) = param.calib.KK*[R T]*ppos(:,i);
    valid(i)=1;
  else
    % push it back on the front side of the image plane, just a tiny bit
    ppos2 = ppos(1:3,i)+R(3,:)'*(-dir+0.01);
    fx(:,i) = param.calib.KK*[R T]*[ppos2;1];
    % it's not a valid point though
  end
  
end

% Divide by the 3rd (homogenous) coordinate and normalize
% by the image size, in order to keep the covariance matrices well-conditioned
%fx = [fx(1,:)./fx(3,:)/param.calib.imageSize(1); ...
%      fx(2,:)./fx(3,:)/param.calib.imageSize(2)];%
fx = [fx(1,:)./fx(3,:); ...
      fx(2,:)./fx(3,:)];

if ~param.repr || param.trim
%  fx(1,:) = max(fx(1,:), -param.calib.imageSize(1));
%  fx(1,:) = min(fx(1,:), +2*param.calib.imageSize(1));
%  fx(2,:) = max(fx(2,:), -param.calib.imageSize(2));
%  fx(2,:) = min(fx(2,:), +2*param.calib.imageSize(2));

  % Which one of the following is better? Ask Rahul about this.

  % Trim to to the boundaries (without regard to the directionality)
  for i=1:size(fx,2)
    n=norm(fx(:,i));
    if n>10^6
      fx(:,i)=fx(:,i)/n*10^6;
    end
  end
  
  % Normalize using the L-infinity norm (i.e. onto a rectangle)
  %fx=fx-0.5;
  %fx=fx./(ones(2,1)*max(max(abs(fx),[],1),1))
  %fx=fx+0.5;
  % what this horrendous expression does is that it computes
  % L-infinity norm for each vector and normalizes the vector
  % only if the norm > 1 (i.e., 0.5 units away from the boundaries)
  % Perhaps we should go further away from the boundaries????  
end
