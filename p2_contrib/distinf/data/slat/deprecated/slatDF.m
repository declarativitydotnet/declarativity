function [xdt, xv, yv, pv, mut, sigmat] = slatDF(problem, k, dist, nCells, interpolate, xp, Pp)

if exist('interpolate', 'var') & interpolate
  disp('Interpolating with a Gaussian at each time step...')
else
  interpolate=0;
end

R = diag([problem.sigma_u problem.sigma_v] ./ ...
    problem.calib(k).imageSize).^2;
%pi = [1:2 model.ci(k,:)];

% Initialize according to the first observation
i=find(problem.visible(k,:));
t=i(1);

% discretize the state first
xv = problem.pos(1,t)+dist*[-1:2/nCells:1];
yv = problem.pos(2,t)+dist*[-1:2/nCells:1];
pv = pi*[-1:1/4/nCells:1];

[xd,yd,pd]=ndgrid(xv,yv,pv);
state=ones(length(xv),length(yv),length(pv));

% initialize the parameter matrices
disp('Initializing camera parameters...')
param1.xi=[1 2 0];
param1.x=[0 0 1.8];
param1.calib=problem.calib(k);
param(1:length(state(:)))=param1;
for i=1:length(state(:)),
  if mod(i,20000)==0
    disp([num2str(i/length(state(:))*100) '% done...'])
  end
  param1.calib.pos(1:2)=[xd(i);yd(i)];
  param1.calib.R = panTilt2R(pd(i), param1.calib.tilt);
  param1.calib.T = -param1.calib.R*param1.calib.pos;
  param(i)=param1;
end

% Minor initializations
R = diag([problem.sigma_u problem.sigma_v] ./ ...
    problem.calib(k).imageSize).^2;

size(state)

ts=find(problem.visible(k,:));
xdt=zeros(length(xv),length(yv),length(pv),length(ts));

for t1=1:8%length(ts),
  t=ts(t1)
  if(problem.visible(k,t))
    if ~exist('xp','var')
      obs = problem.obs(:,t,k)./(problem.calib(k).imageSize');
      %obs = problem.projection(1:2,t,k)./(problem.calib(k).imageSize');
      pos = problem.pos(1:2,t);
      P=(0.10^2)*eye(2);
    else
      obs = problem.obs(:,t,k)./(problem.calib(k).imageSize');
      pos = xp(1:2,t);
      P   = Pp(1:2,1:2,t);
    end
    %state(find(state<1e-20))=0;
    is=find(state>1e-5);
    [muY, sigmaY, valid]=approximateGaussian(pos, P, ...
        @fnFixedProjection, R, param(is));
    state(is)=state(is).*(valid');
    vs=find(valid);
    is=is(vs);
    state(is)=state(is).*mvnpdf(obs', muY(:,vs)', sigmaY(:,:,vs));
  end

  xdt(:,:,:,t1)=state;
  [mut(:,t1) sigmat(:,:,t1)]=discrete2gaussian(state, xv, yv, pv);
  if interpolate,
    state(:)=mvnpdf([xd(:) yd(:) pd(:)], mut(:,t1)', ...
        sigmat(:,:,t1));
  end
end
