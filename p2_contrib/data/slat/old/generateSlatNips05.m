function prob=generateSlatNips05(simulation, halfsize)
% prob=generateSlatIntel(simulation)
%   or
% prob=generateSlatIntel(simulation, ids)

if ~exist('halfsize', 'var')
  halfsize=0;
end

height = 2.25 * 0.0254; %height of the marker

% Extrinsic parameters: x,y,z (inches), pan, tilt (radians)
ext = [
  120   12   9.5   0   -0.4; ...       51
  100    8   7.5   pi/2   -0.4; ...       52
  60    8   9.5   pi/2   -0.4; ...       53
  20     8   8.5   pi/2   -0.4; ...       54
  120   64  8.5  -pi/2   -0.4; ...       55
  80    64  8.5  -pi/2   -0.4; ...       56
  42    64  8.5  -pi/2   -0.4; ...       57
  8     64  8.5  -pi/2   -0.4]';%        58

 
ext(1:3,:)=ext(1:3,:)*0.0254;

% Intrinsic parameters
load('firei');
%...
% [461.4538         0  329;...
%         0  461.0651  240;...
%         0         0    1.0000];
ids=51:58;
KK=KK(ids);

if halfsize
  for i=1:length(KK), 
    KK{i}(1:2,1:3)=KK{i}(1:2,1:3)/2; % To account for the fact that images are 320 x 240
  end
  imgsize = [320 240];
else
  imgsize = [640 480];
end  
    
calib=generateCalibration(KK, ext, imgsize);
range=[-1.5 1.5 -1.5  1.5];


if simulation
  prob=generateSlatProblem(calib, 'range', range, 'height', height);
else
  prob.nCams=length(calib);
  prob.calib=calib;
  prob.range=range;
  prob.sigma_u=40;
  prob.sigma_v=40;
  prob.height=height;
end
prob.cameraid = ids;

%   prob.geometry = ...
%         {[0 0; -12 0; -12 -9; 0 -9; pi/2 -pi/2]};
order = [1 3 5 7 2 4 6 8];
%        [1 5 2 6 3 7 4 8];
adj = generateSlatAdjacency(8,1);
prob.adjacency=adj(order, order);
