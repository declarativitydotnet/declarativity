function [a,b]=plotSlatObservation(prob, time_step, cameras,soln);
%[a,b]=plotSlatObservation(prob, time_step, cameras);

if ~exist('cameras', 'var')
  cameras=1:prob.nCams;
end

ncams=length(cameras);
a=sqrt(ncams/(4/3));
b=ceil(a*4/3);
a=ceil(ncams/b);
cla;
for i=1:ncams,
  k=cameras(i);
  subplot(a,b,i);
  if isfield(prob, 'imgfile') && exist(prob.imgfile{k, time_step}, 'file')
    im=imread(prob.imgfile{k, time_step});
    im=im(1:2:end,1:2:end,1:3);
  elseif isfield(prob, 'mpgfile') && exist(prob.mpgfile{k}, 'file')
    mov=mpgread(prob.mpgfile{k}, min(time_step+1,prob.nSteps-2), 'truecolor');  
    % adding +1 to time step b/c the decoding or transcoding seems to add
    % an extra frame at the beginning
    im=mov.cdata;
  else
    im=ones(30,40)*0.5;
  end
  imshow(im);
  if prob.visible(k,time_step)
    hold on;
    obs=prob.obs(:,time_step,k)'./prob.calib(k).imageSize.*[size(im,2) size(im,1)];
    plot(obs(1), obs(2), 'r*');
    if exist('soln','var')
      muX = soln.xt(1:2,time_step);
      sigmaX=full(soln.Pt{time_step}(1:2,1:2));
      param.repr=0;
      param.calib=prob.calib(k);
      param.height=prob.height;
      [muY, sigmaY] = approximateGaussian(muX, sigmaX, @fnProjection, zeros(2), param);
      A=diag([size(im,2) size(im,1)]./prob.calib(k).imageSize);
      muY=A*muY;
      sigmaY=A*sigmaY*A';
      plotcov2(muY,sigmaY,'plot-axes',0,'plot-opts',{'color','w'});
      
%       param.repr=0;
%       param.calib=prob.calib(k);
%       param.calib.R=prob.R{k};
%       param.height=prob.height;
%       [muY, sigmaY] = approximateGaussian(muX, sigmaX, @fnProjection, zeros(2), param);
%       A=diag([size(im,2) size(im,1)]./prob.calib(k).imageSize);
%       muY=A*muY;
%       sigmaY=A*sigmaY*A';
%       plotcov2(muY,sigmaY,'plot-axes',0,'plot-opts',{'color','w'});
    end
    hold off;
  end
  if isfield(prob, 'cameraid')
    title(['Camera ' num2str(prob.cameraid(k))]);        
  else
    title(['Camera ' num2str(k)]);
  end
end
