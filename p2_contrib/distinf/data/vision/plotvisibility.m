function h = plotvisibility(calib, height, fillcolor, varargin)
% h=plotvisibility(calib, varargin)

p = -calib.R'*calib.T;
plane = [0 0 1 -height];
points = [0 0 calib.imageSize(1)-1 calib.imageSize(1)-1;
  0 calib.imageSize(2)-1 calib.imageSize(2)-1 0];
vis = invert_camera_projection(points, calib, plane);
for j=1:4,
  diff = p(1:3) - vis(1:3,j);
  if calib.R(3,:)*diff > 0,
    vis(1:2,j) = vis(1:2,j)+100 / norm(diff(1:2))*diff(1:2);
  end
end
if length(fillcolor)>0
  h = fill(vis(1,:), vis(2,:), fillcolor);
else
  h = line([vis(1,:) vis(1,1)],[vis(2,:) vis(2,1)]);
end
set(h,varargin{:});
