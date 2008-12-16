function prob=extractSlatObservations(ncameras, basename, time)
prob.nSteps = length(time);
prob.nCams  = ncameras;
visible = zeros(prob.nCams,prob.nSteps);
obs=zeros(2,prob.nSteps,prob.nCams);
sigma=zeros(2,prob.nSteps,prob.nCams);

for k=1:ncameras,
  % Compute the background as the average of all the frames
  disp(['Computing background for camera ' num2str(k) '...'])
  background=double(imread([basename '_' num2str(1) '_' num2str(k-1) '.jpg']));
  for i=2:prob.nSteps,
    background=background+double(imread([basename '_' num2str(i) '_' num2str(k-1) '.jpg']));
  end
  background=background/prob.nSteps;
  
  % Now compute the observations
  for i=1:prob.nSteps,
    im=imread([basename '_' num2str(i) '_' num2str(k-1) '.jpg']);
    [changed,pixels]=subtractSimple(background,im,64);
    imshow(pixels);
    [center,dim]=extractObjects(changed,9,1);
    if size(center,1)>1
      warning(['Camera ' num2str(k) ' detected more than one object '...
               'at t=' num2str(i) ', ignoring...']);
    end
    if size(center,1)==1
      hold on;
      plot(center(1),center(2),'ro');
      hold off;
      visible(k,i)=1;
      obs(:,i,k)=center';
      sigma(:,i,k)=dim'/2;
    end;
    drawnow;
  end
end
prob.visible=visible>0;
prob.obs=obs;
prob.sigma=sigma;
prob.time=time;

