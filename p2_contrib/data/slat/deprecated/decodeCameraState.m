function [cpos, RR, TT] = decodeCameraState(x, param)
% [cpos, RR, TT] = decodeCameraState(x, param)
%   Decodes the state representation and fills in any constants from 
%   param.calib into camera parameters.
% x:     person/camera state
% param  the state description
% cpos:  camera center positions (3 x n)
% RR:    camera rotation matrices (3 x 3 x n)
% TT:    camera tranform matrices (3 x n)

if param.repr~=0 && param.repr~=1 && param.repr~=2
  error('Unknown parametrization of the camera calibration');
end

[stuff, nx] = size(x);

if param.repr==0,
  param.cposi=zeros(3,1);
  param.pani=0;
  param.tilti=0;
  param.trim=0;
end

for i=1:3,
  if param.cposi(i)
    cpos(i,:) = x(param.cposi(i),:);
  else
    cpos(i,:) = param.calib.pos(i)*ones(1,nx);
  end
end

if param.repr==2,
  for i=1:nx,
    pan = x(param.pani, i);
    R = [cos(pan) sin(pan); -sin(pan) cos(pan)];
    cpos(1:2,i) = cpos(1:2,i) + R * x(param.cposi(1)+[2 3],i);
  end
end

RR = zeros(3, 3, nx);
TT = zeros(3, nx);

for i=1:nx,
  if param.pani
    pan = x(param.pani, i);
  else
    pan = param.calib.pan;
  end

  if param.tilti
    tilt = x(param.tilti, i);
  else
    tilt = param.calib.tilt;
  end

  if param.repr==0,
    RR(:,:,i)=param.calib.R;
  else
    RR(:,:,i) = panTilt2R(pan, tilt);
  end
  TT(:,i) = -RR(:,:,i)*cpos(:,i);
end
