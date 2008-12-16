function prob=generateSlatPHB(prob)
% problem=generateSlatPHB(basename, time)     or
% problem=generateSlatPHB(problem_obs)

  nCams=8;

if exist('prob', 'var');
  prob.nCams=nCams;
  prob.nSteps=size(prob.visible,2);
  simulation=0;
else
  simulation=1;
end

n=nCams;

% Extrinsic parameters: x,y,z (inches), pan, tilt (radians)
ext1 = [
  0.5+(1:n)*0.5; ...
  1.2*(-ones(1,n)).^(1:n); ...
  0.25*ones(1,n); ...
  -pi/2*(-ones(1,n)).^(1:n); ...
  -0.4*ones(1,n)];
ext2=[];
%ext2=[0 0 0.3 -pi -0.42]';

%ext = [-82 -272  112.5 0         -0.3932; ...
%       -82 -176  112.5 0         -0.4110; ...
%       -82 -80   112.5 -pi/2/6   -0.3744; ...
%       -88 -17   97    -pi/2*2/3 -0.3127; ...
%       -298 -140 105   pi        -0.4124; ...
%       -296 -206 108.5 pi        -0.3470]';

%ext(1:3,:)=ext(1:3,:)*0.0254;

% Intrinsic parameters
KK = ...
 [766.4538         0  329;...
         0  763.0651  240;...
         0         0    1.0000];


%for i=1:length(KK), 
%  KK{i}(1:2,1:3)=KK{i}(1:2,1:3)/2; % To account for the fact that images are 320 x 240
%end
    
% calib=generateCalibration(KK, ext, [320 240]);
calib=generateCalibration(KK, [ext1 ext2], [640 480]);

if simulation
  prob=generateSlatProblem(calib, 'range', [0 5 -1.25  1.25], 'height', 0, 'fps', 5);
  %prob.geometry = ...
	%}
  %
  %    {[0 0; -12 0; -12 -9; 0 -9; pi/2 -pi/2]};
else
  % The first camera to see the person
  i = find(prob.visible(:,1));
  if length(i)==0
    error('The person is not visible from any cameras in the first time step.');
  else
    i=i(1);
  end

  prob.pos=invertProjection(prob.obs(:,1,i),calib(i),0,0,[0 0 1 0]);
  prob.calib=calib;
  prob.range=[-14 2 -11 2];
  prob.sigma_u=3;
  prob.sigma_v=3;
  prob.time=0.1:1/10:prob.nSteps/10;
  prob.geometry = ...
        {[0 0; -12 0; -12 -9; 0 -9; pi/2 -pi/2]};
  prob.adjacency=[1 1 0 1 0 1; 1 1 1 1 1 1; 0 1 1 1 1 0; 1 1 1 1 1 1; 0 1 1 1 1 1; 1 1 0 1 1 1];
end

prob.adjacency=generateSlatAdjacency(nCams,1,1);

