function animateData(ncameras, basename, time, prob)
side = ceil(sqrt(ncameras));

for i=1:length(time)
  for k=1:ncameras
    subplot (side,side,k);
    im=imread([basename '_' num2str(i) '_' num2str(k-1) '.jpg']);
    imshow(im);
    if exist('prob','var') & prob.visible(k,i)
      hold on;
      plot(prob.obs(1,i,k),prob.obs(2,i,k),'ro');
      hold off;
    end
    drawnow;
  end
end
