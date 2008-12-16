function P = invert_camera_projection(p, calib, varargin)
% P=invert_camera_projection(p, calib, varargin)
% varargin: plane to project on (in the standard format ax+by+cz+d=0)
%

if length(varargin)==0
  plane=[0 0 1 0];
else
  plane=varargin{1};
end

M = calib.KK * [calib.R calib.T];
M1= [M; plane];
p(3,:) = 1;
p(4,:) = 0;
P = M1 \ p;
P = P(1:3,:)./(ones(3,1)*P(4,:));
