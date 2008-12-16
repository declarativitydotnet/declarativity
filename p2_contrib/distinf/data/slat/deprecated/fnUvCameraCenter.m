function [fx,valid] = fnUvCameraCenter(x, param)
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
valid=ones(1,nx);

cpos = decodeCameraState(x, param);
fx=cpos(1:2,:);


