function newprob=loadSlatObservations(problem, path)
% newprob=loadSlatObservations(problem, path)
newprob=problem;

% Load the experiment parameters 
[newprob.start, newprob.duration, newprob.fps] = ...
  load_options([path '/experiment.conf'], 'start', ' ', 'duration', 0, 'fps', 0);

% TODO: fix when the duration is not known a priori

nloaded=0; % # of cameras, for which we loaded observations
ncams = problem.nCams;
obs=zeros(2,0,ncams);
visible=zeros(ncams,0);
mpgfile=cell(ncams,1);

for i=1:ncams,
  k=problem.cameraid(i);
  detfilename=sprintf('%s/cam%02d.det',path,k);
  if exist(detfilename,'file')
    data=load(detfilename)';
    %data=data(:,2:end);  % ignore the first observation -- will mess up the video!
    nobs=size(data,2);
    if nobs>0
      visible(i,1:nobs)=(data(1,:)>-1);
      obs(:,1:nobs,i)=data;
    end
    nloaded=nloaded+1;
  end

  mpgfilename=sprintf('%s/cam%02d.mpg',path,k);
  if exist(mpgfilename,'file')
    mpgfile{i}=mpgfilename;
  end
end
newprob.nSteps=size(visible,2);
newprob.time=(1:newprob.nSteps)/newprob.fps;
newprob.visible=logical(visible);
newprob.obs=obs;
newprob.mpgfile=mpgfile;
i = find(visible(:,1));
if length(i)==0
  warning('The person is not visible from any cameras in the first time step.');
  newprob.pos=[0;0;problem.height];
else
  i=i(1);
  newprob.pos=invertProjection(obs(:,1,i), problem.calib(i),0,0,[0 0 1 -problem.height]);
end
