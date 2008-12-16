function problem=generateSlatGrid(KK,xs,ys,height,adjacency_range,varargin)
% problem=generateSlatOverhead(KK,xs,ys,height,adjacency_range,varargin)
% the optional parameters fps, mps are passed to generateSlatProblem
% Generates a grid of cameras
  
[x,y]=meshgrid(xs,ys);
n=length(x(:));
%A grid of cameras at x,y,height, oriented in random directions, facing
%down
pos=[x(:)';y(:)';ones(1,n)*height;rand(1,n)*2*pi;ones(1,n)*(-pi/2)];
calib=generate_calibration(KK, pos);
range=[min(xs)-2 max(xs)+2 min(ys)-2 max(ys)+2];
problem = capture_trajectory(calib, 'range', range, varargin{:});
dx=x(:)*ones(1,n)-ones(n,1)*(x(:)');
dy=y(:)*ones(1,n)-ones(n,1)*(y(:)');
if length(adjacency_range)==1
  problem.adjacency=double(sqrt(dx.^2+dy.^2)<=adjacency_range);
else
  problem.adjacency=double(abs(dx)<=adjacency_range(1) & ...
                           abs(dy)<=adjacency_range(2));
end
