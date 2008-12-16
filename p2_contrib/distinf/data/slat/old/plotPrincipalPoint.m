function plotPrincipalPoint(prob, basedir)
for i=1:length(prob.calib),
  mydir=[basedir num2str(i)];
  files=dir([mydir '/*.jpg']);
  [mydir '/' files(1).name]
  im=imread([mydir '/' files(1).name]); % warning: might be a directory
  imshow(im);
  hold on;
  center=prob.calib(i).KK(1:2,3);
  plot(center(1),center(2),'or');
  hold off;
  input('Press Enter to continue to the next image');
end
