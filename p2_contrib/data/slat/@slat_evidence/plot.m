function [a,b]=plot(ev, varargin)
% [a,b]=plot(ev, ...) -- plot the camera observations from all time steps
% [a,b]=plot(ev, time_step, ...) -- plot the observations in 1 time step

if length(varargin)>0 && isnumeric(varargin{1})
  time_step = varargin{1};
  varargin = varargin(2:end);
else
  time_step = [];
end

tb = testbed(ev);
visible = get(ev, 'visible');
obs = get(ev, 'obs');
dat = get(ev, 'data');

cameras = process_options(varargin, 'cameras', 1:length(tb.calib));

ncams=length(cameras);
a=sqrt(ncams/(4/3));
b=ceil(a*4/3);
a=ceil(ncams/b);
calib=tb.calib;
  
if length(time_step)==0
  clf;
  steps = [1 20:20:nsteps(ev)];
  for i=1:ncams,
    k=cameras(i);
    subplot(a,b,i);

    axis equal;
    axis([0 calib(k).imageSize(1)-1 0 calib(k).imageSize(2)-1]);
    set(gca,'YDir', 'reverse');
    hold on;
    grid on;
    %if isfield(problem,'projection')
    %  plot(problem.projection(1,:,k), problem.projection(2,:,k),'ro-');
    %end
    obs2=obs(:,:,k);
    obs2(:,find(visible(k,:)==0))=nan;
    plot(obs2(1,:),obs2(2,:), 'mo-');
    %if exist('exalib', 'var') & isfield(problem, 'pos')
    %  p = ecalib.KK*[ecalib(k).R ecalib(k).T] * ...
    %      [problem.pos; ones(1,nsteps(ev))];
    %  p = p./(ones(3,1)*p(3,:));
    %  plot(p(1,:), p(2,:), 'go-');
    %end

    %legend('Exact projection', 'Observation', 'Exact w/ ecalib');
    for t=steps,
      if visible(k,t)
        h=text(obs(1,t,k)+5,obs(2,t,k)-5,num2str(t));
        set(h,'backgroundcolor',[1 1 1]);
      end
    end
    title(camera(tb, k));
    hold off;
  end

else
      clf;
  for i=1:ncams,
    k=cameras(i);
    subplot(a,b,i);
    

    
    %if isfield(prob, 'imgfile') && exist(prob.imgfile{k, time_step}, 'file')
    %  im=imread(prob.imgfile{k, time_step});
    %  im=im(1:2:end,1:2:end,1:3);
    %elseif isfield(prob, 'mpgfile') && exist(prob.mpgfile{k}, 'file')
    %  mov=mpgread(prob.mpgfile{k}, min(time_step+1,prob.nSteps-2), 'truecolor');  
    %  % adding +1 to time step b/c the decoding or transcoding seems to add
    %  % an extra frame at the beginning
    %  im=mov.cdata;
    %else
    %  im=ones(30,40)*0.5;
    %end
    
    dims = plot_image(dat, k, time_step);

%    im=ones(30,40)*0.5;
%    imshow(im);
    if visible(k,time_step)
      hold on;
      obs1=obs(:,time_step,k)'./(calib(k).imageSize).*dims;
      plot(obs1(1), obs1(2), 'r*');
      hold off;
    end
    title(camera(tb, k));
  end
end
