function model = generateSlatModel(problem, xy, height, pan, tilt, dynamic)
% model = generateSlatModel(problem, xy, height, pan, tilt, dynamic)
% Generates a model for a set of n cameras, using the features 
% specified.
% 
% The model is linear for the state and projective for the observations:
%   x(t)   = A(t)x(t-1) + N(0,Q(t))
%   y_k(t) = projection(x(t)<person location, camera parameters>)+N(0,R)
%
% where A(t) = model.A+model.Adt*dt;
%       Q(t) = model.Q+model.Qdt*(dt^2);
%       x<v> is vector x restricted to variables v
%       and R is specified by the problem
%
% Parameters:
%   xy      to turn on camera 2D position estimation,
%   height  to turn on camera height estimation,
%   pan     to turn on camera pan estimation, with mean=true pan
%   tilt    to turn on camera tilt estimation
%   dynamic use a dynamic model of the person's motion
%
% parameter=1 => initialize with a focused prior
% parameter=2 => initialize with a wide prior
%
% Default is 0 on all.

% Set the default values
vars={'xy', 'height', 'pan', 'tilt', 'dynamic'};
for i=1:length(vars),
  if ~exist(vars{i},'var')
    eval([vars{i} '=0;']);
  end
end

% Environment size (the standard deviation for x,y)
worldSize = 100;

% The number of cameras; default camera parameters;
calib = problem.calib;
model.nCams = length(calib);

if dynamic,
  %x0  = zeros(4,1);
  x0  = [problem.pos(1:2,1);0;0];
  P0  = diag([1e-4 1e-4 1 1]);
  A   = eye(4);
  Adt = [0 0 1 0; 0 0 0 1; zeros(2,4)];
  Q   = zeros(4);
  Qdt = diag([0,0,0.3,0.3]); % person can accelerate at sqrt(0.3)m/s^2

  model.vpp   = 4;
  model.pVars = {'px' 'py' 'vx' 'vy'};
  model.pi    = 1:4;
  
else % static model
  %x0  = zeros(2,1);
  x0  = [problem.pos(1:2,1)];
  P0  = eye(2)*(0.01^2);
  A   = eye(2);
  Adt = zeros(2);
  Q   = zeros(2);
  Qdt = eye(2); % The person "spreads" uniformly at 1m/s
  
  model.vpp   = 2; % number of latent variables
  model.pVars = {'px' 'py'};
  model.pi    = 1:2;
end


% Initialize the information about state variables for cameras
hidden  = [xy~=0 xy~=0 height~=0 xy==3 xy==3 (pan==1 || pan==2) pan==3 pan==3 tilt~=0];
names   = {'cx' 'cy' 'cz' 'u' 'v' 'pan' 'cpan' 'span' 'tilt'};
varpos  = (2+cumsum(hidden)).*hidden;

model.vpc   = sum(hidden);
model.cVars = {names{hidden}};
model.ci    = reshape(model.vpp+(1:(model.nCams*model.vpc)), ...
    model.vpc, model.nCams)';

model.vars  = model.pVars;
model.xt    = zeros(size(model.ci,1),1);
for k=1:model.nCams,
  pank = problem.calib(k).pan;
  camtruth=[problem.calib(k).pos; pank; cos(pank); sin(pank); ...
            problem.calib(k).tilt];
  model.xt(model.ci(k,:))=camtruth(find(hidden));
  for i=1:model.vpc,
    model.vars{model.ci(k,i)} = [model.cVars{i} num2str(k)];
  end
end

% Initialize the information for computing projections
if xy==3
  param.repr = 2;
else
  param.repr = 1;
end

param.cposi = varpos(1:3);
param.pani = varpos(6)+varpos(7);
param.tilti = varpos(9);
param.trim = 0;
if isfield(problem, 'height')
  param.height=problem.height;
else
  param.height=1.8;
end
for k=1:model.nCams,
  param.calib = calib(k);
  model.param(k) = param;
end


% Set the initial state estimate and transition model for the
% cameras

% The initial means and standard deviations for the different
% camera parameters and accuracy of initial distribution
means = [nan       nan       nan 0 0 nan  0 0 nan; ...
         0         0         3   0 0 0    0 0 0.5];

stds  = [2         2         0.1 1 1 pi/5 1 1 0.4; ...
         worldSize worldSize 1   worldSize worldSize pi   1 1 0.5];
hiddeni = find([xy==1 xy==1 height==1 0 0 pan==1 0 0 tilt==1; ...
      xy>=2 xy>=2 height==2 xy==3 xy==3 pan==2 pan==3 pan==3 tilt==2]);

foundVisible = 0;

for k=1:model.nCams,
  pank = calib(k).pan;
  means(1,:) = [calib(k).pos' 0 0 pank cos(pank) sin(pank) calib(k).tilt];
  ci = model.ci(k,:);

  if pan==2 & problem.visible(k,1) & foundVisible==0
    % Initialize a mean with true direction
    foundVisible = 1;
    means2 = means;
    stds2  = stds;
    means2(2,6) = calib(k).pan;
    stds2 (2,6) = 1e-4;
    %means2(2,1:2)=calib(k).pos(1:2)'; %hack hack
    %stds2 (2,1:2)=1e-4;
    x0(ci)     = means2(hiddeni);
    P0 (ci,ci) = diag(stds2(hiddeni).^2);
  elseif pan==3 & problem.visible(k,1) & foundVisible==0
    % special pan
    foundVisible=1;
    means2 = means;
    stds2 = stds;
    means2(2,7)=cos(calib(k).pan);
    means2(2,8)=sin(calib(k).pan);
    stds2(2,7:8)=1e-6;
    x0(ci)     = means2(hiddeni);
    P0 (ci,ci) = diag(stds2(hiddeni).^2);
  else  % regular mean
    x0(ci)     = means(hiddeni);
    P0 (ci,ci) = diag(stds(hiddeni).^2);
  end
  
  A  (ci,ci) = eye(model.vpc);
  Adt(ci,ci) = zeros(model.vpc);
  Q  (ci,ci) = zeros(model.vpc);
  Qdt(ci,ci) = zeros(model.vpc);
end

model.x0  = x0;
model.P0  = P0;
model.A   = A;
model.Adt = Adt;
model.Q   = Q;
model.Qdt = Qdt;
