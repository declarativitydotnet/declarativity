function [prob,model,soln]=slatExperiment(path, stage)
cd /home/sfuniak/cvs/camcontrol/matlab
%detector_params='-0.3959 0.7801 -0.4846 0.075' % Indoor lights
%for i=1:4, detector_params{i}='rundetector -0.5808 0.7743 -0.2512 0.080';end; % Away from window
%for i=5:6, detector_params{i}='-0.5619 0.7724 -0.2960 0.040';end; % Facing window
%for i=5:6, detector_params{i}='rundetector_hsv 0.3 0.45 0.2 1';end; % Facing window
for i=1:6, detector_params{i}='rundetector_hsv 0.25 0.45 0.2 1';end; 
for i=4:4, ignore_params{i}=''; end;
ignore_params{1}='210 -1 305 36';
ignore_params{2}='115 -1 213 33';
ignore_params{3}='112 -1 202 42';
ignore_params{5}='150 6 278 68 272 78 306 104';
ignore_params{6}='100 18 248 122 264 2 321 88 64 85 95 109';

ncams=6;
lispdir='~/svn/stroj/gm/lisp';

if ~exist('stage', 'var')
  stage=1;
end

mkdir(path);
!rm ~/experiment
eval(['!ln -s ' path ' ~/experiment']);

if stage<=1  % Setup the directories for camsource and collect the data
  for i=1:ncams,
    mkdir([path '/cam' num2str(i)]);
  end

  % Collect the data
  !../camsource-0.7.0/runcamsource &
  input(['Collecting data, press ENTER to finish...']);
  !camsource -k
end


if stage<=2 % Extract the observations and generate the problem instance
  
  obs=zeros(2,0,ncams);
  visible=zeros(ncams,0);
  for i=1:ncams,
    disp(['Extracting observations from camera ' num2str(i) '...'])
    eval(['!../detector/' detector_params{i} ' ' ignore_params{i} ' ' ...
      path '/cam' num2str(i) '/*.jpg > observations.txt']);
    data=load('observations.txt')';
    nobs=size(data,2);
    visible(i,1:nobs)=(data(1,:)>-1);
    obs(:,1:nobs,i)=data;
  end
  prob.visible=logical(visible);
  prob.obs=obs;
  nsteps=size(prob.visible, 2);
  prob.imgfile=cell(ncams, nsteps);
  for i=1:ncams,
    for j=1:nsteps,
      prob.imgfile{i,j}=[path '/cam' num2str(i) '/' sprintf('%06d', j) '.jpg'];
    end
  end
  save workspace
  
  prob=generateSlatPHB(prob);
  model=generateSlatModel(prob,1,0,1,0,1)
  plotSlatObservations(prob);
  drawnow;
  save('~/experiment/prob', 'prob', 'model');
  saveSlatProblem(prob, '~/experiment/prob.lisp');
end

if stage<=3 % Run the simulation 
  !cd ~/svn/stroj/gm/lisp;~/bin/a/alisp -L slat/run-phb.lisp
end

if stage<=4 % Load results from .lisp and save the output
  load ~/experiment/prob
  soln=loadSlatResults('~/experiment', 'rc');
  save('~/experiment/soln', 'prob', 'model', 'soln');
end
