function dims = plotimg(prob, camera, time_step)
% function dims = plotimg(prob, camera, time_step)
% The API should really be time, rather than time_step

if isfield(prob, 'mpgfile')
  % && exist(mpgfile{camera}, 'file')

  mpgfile = prob.mpgfile;
  nsteps = prob.duration * prob.fps;

  mov=mpgread(mpgfile{camera}, min(time_step+1, nsteps-2), 'truecolor');  
  % adding +1 to time step b/c the decoding or transcoding seems to add
  % an extra frame at the beginning
  im = mov.cdata;
  imshow(im);
  
  dims = size(im); dims=dims([2 1]);

else

  calib = prob.calib(camera);
  rectangle('position', [0 0 calib.imageSize], 'facecolor', 'w');
  axis equal;
  axis([0 calib.imageSize(1)-1 0 calib.imageSize(2)-1]);
  axis off;
  set(gca,'YDir', 'reverse');
  dims = calib.imageSize;

end

% plot the observations
hold on
obs = prob.obs{camera};
plot(obs(1,:), obs(2,:), 'ro');
steps=[1 prob.nsteps];
%set(h, 'MarkerSize', 10);
for t=steps,
  h=text(obs(1,t), obs(2,t)+0.05, ['\it{t} = ' num2str(t)]);
  set(h, 'verticalalignment', 'baseline');
end
hold off
