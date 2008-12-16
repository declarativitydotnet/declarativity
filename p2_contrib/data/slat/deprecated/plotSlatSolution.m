function plotSlatSolution(prob, model, soln, t, options)
% plotSlatSolution(prob, model, soln, t, options)
% Plots the outcome of the estimation
%
% General options from plotSlatProblem:
%   'c' - camera centers 
%   'd' - camera centers and orientations
%   'v' - camera visibility range
%   't' - trajectory of the marker (if available)
%   'o' - camera observations
%   'f' - use the current figure, rather than Figure 1
%   'g' - geometry
%   'p' - optimize for publication (larger fonts etc.)
%
% Options specific to plotSlatSolution:
%   'e' - estimated camera centers and person location
%   'l' - plot the actual person's location (if available)
%   'p' - plot a partial trajectory of the marker up to time t
%   'q' - plot the person estimate from all the cameras
%   'n' - network junction tree (if available)
%   'r' - apply the optimal rigid transform (if available)
%
% defaults: 'defa'
% optional plots:
%          'etraj'   - estimated trajectory of the person
%          'ecalib'  - estimated camera calibrations
%          'ecenter' - estimated camera centers (means, covariances)
%          'epos'    - estimated person position (mean, covariance)
%          'tree'    - the tree for cameras

if ~exist('options', 'var')
  options = 'defa';
end

if ~isfield(soln, 'time')
  soln.time=prob.time;
end

% Plot the problem 
plotSlatProblem(prob, options);
if ~any(options=='p')
  title(sprintf('t = %.2fs, step=%d', soln.time(t), t));
end
hold on;

% Get some common statistics
if any(options=='r')
  if isfield(soln, 'transform')
    transform=soln.transform;
  else
    transform=slatComputeOptimalTransform(prob, model, soln);
  end
else
  transform=struct('R', eye(2), 'c', 1, 't', zeros(2,1));
end
R=transform.R;
tt=transform.t;
c=transform.c;

xt=soln.xt(:,t);
Pt=soln.Pt{t};

% Plot the actual and estimated trajectory
%if any(options=='p')
%  h=plot(etraj(1,:),etraj(2,:),'g-');
%  set(h, 'linewidth', 4);
%  for t=steps,
%    h=text(prob.pos(1,t)+0.1,prob.pos(2,t)-0.1,num2str(t));
%    set(h,'backgroundcolor',[1 1 1]);
%  end
%end

% Plot the actual marker location
if any(options=='l') & isfield(prob, 'pos') & size(prob.pos, 2)>=t
  plot(prob.pos(1,t), prob.pos(2,t), 'kx');
end

% Plot the estimated camera centers
nstdevs=-norminv(0.05/2); % 95% confidence interval in a normal distrib.
if any(options=='e')
  for k=1:prob.nCams,
    ci = [1:2 model.ci(k,:)];
    if Pt(ci(3),ci(3))<9000, %hack hack hack
      center=approximateAngleDistribution(xt(ci),full(Pt(ci,ci)),...
        @fnUvCameraCenter, eye(2)*1e-6, model.param(k));
      for j=1:length(center.p),
        center.mus(:,j)=c*R*center.mus(:,j)+tt;
        center.covs(:,:,j)=c^2*R*center.covs(:,:,j)*R';
      end

      if prob.visible(k,t)
        plot_opts = {'m'};
        plot(prob.calib(k).pos(1),prob.calib(k).pos(2),'mx');
      else
        plot_opts = {'b'};
      end

      % Plot the position confidence bound
      h=plotMogError(center, 0.95, plot_opts);
      set(h, 'linewidth', 2);

      % Plot the pan confidence bound
      [mu, sigma]=mixture2gaussian(center);
      ci = model.ci(k,model.param(k).pani-2);
      meanT=pi-(xt(ci)+atan2(transform.R(1,2),transform.R(1,1))); 
      sigmaT=sqrt(Pt(ci,ci));
      h=quiver([mu(1);mu(1)],[mu(2);mu(2)], ...
        [cos(meanT+nstdevs*sigmaT);cos(meanT-nstdevs*sigmaT)]/2, ...
        [sin(meanT+nstdevs*sigmaT);sin(meanT-nstdevs*sigmaT)]/2,0);
      set(h,'maxheadsize',3);
      set(h,'showarrowhead','off');
      set(h,'showarrowhead','on');
    end
  end
end
  
% Plot the estimated person location
if any(options=='q') % plot the estimates from all the nodes
  for k=1:prob.nCams,
    plotcov2(c*R*soln.xtl{k}(:,t)+tt, c^2*R*soln.Ptl{k}(:,:,t)*R', ...
      'conf', 0.95, 'plot-opts', {'r'}, 'plot-axes', 0);
  end
elseif any(options=='e') 
  h=plotcov2(c*R*xt(1:2)+tt, c^2*R*full(Pt(1:2,1:2))*R', ...
    'conf', 0.95, 'plot-opts', {'r'}, 'plot-axes', 0);
  set(h, 'linewidth', 2);
end

% Plot the network junction tree
if any(options=='n') & length(soln.ttree)>0
  % Find the latest tree for this time step
  itrees=find(soln.ttree<=soln.time(t));
  tree=soln.tree(itrees(end),:);
  ca=cos(pi/8);
  sa=sin(pi/8);

  if(sum(tree==0)>1)
    color='r';
  else
    %if any(options=='p')
    %  color='k';
    %else
      color=[0 0.6 0];
    %end
  end

  for i=1:length(tree),
    to=tree(i);
    if to>0
      posi=prob.calib(i).pos(1:2);
      posj=prob.calib(to).pos(1:2);
      posr=posj+[ca sa; -sa ca]*(posi-posj)/norm(posi-posj)*0.3;
      posl=posj+[ca -sa; sa ca]*(posi-posj)/norm(posi-posj)*0.3;
      h=line([posi(1);posj(1)],[posi(2);posj(2)]);
      set(h,'color',color);
      if any(options=='p')
        set(h,'LineWidth',2);
      end
      h=patch([posj(1);posr(1);posl(1)],[posj(2);posr(2);posl(2)],color);
      set(h,'edgecolor', color);
    else
      text(prob.calib(i).pos(1)-0.2,prob.calib(i).pos(2)-0.2,'r');
    end
  end
end

hold off;
drawnow;
  





% Old plots
%
% xt=solution.xt;
% nFigs = plotSlatProblem(prob, 'cvt', 'etraj', xt(1:2,:)); %, 'ecalib', ecalib);
% ti = 1:floor(prob.nSteps/40):prob.nSteps;
% 
% figure(nFigs+1);
% plot(xt(model.pi,:)');
% hold on;
% [vel, acc]=computeMotionStatistics(prob);
% plot([prob.pos(1:2,1:end-1); vel]','--');
% legend({model.pVars{:} 'true px' 'true py' 'true vx' 'true vy'});
% for i=1:length(model.pi)
%   v = model.pi(i);
%   errorbar(ti, xt(v, ti), 2*sqrt(Pt{ti}(v,v)),'ok');
% end
% hold off;
% 
% for k=1:model.nCams,
%   legendText{k}=['Camera ' num2str(k)];
% end
% 
% for i=1:model.vpc,
%   figure(nFigs+1+i)
%   plot(xt(model.ci(:,i),:)');
%   hold on;
%   for k=1:model.nCams,
%     v = model.ci(k,i);
%     err=2*sqrt(Pt{ti}(v,v));
%     errorbar(ti, xt(v, ti), err.*(err<20),'ok');
%   end
%   hold off;
% 
%   %legend(legendText);
%   xlabel('time step');
%   ylabel(model.cVars{i});
% end
% 
