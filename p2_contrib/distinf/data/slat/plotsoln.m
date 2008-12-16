function transform=plot(solns, varargin)
% function plot(soln, ...)
% function plot(soln, t, ...)
% function plot(soln, dm, ...)
%
% Options: printable 0
%          ralign    1
%          object    3 (bit 0: groundtruth, bit 1: estimate)
%          cameras   3 (bit 0: groundtruth, bit 1: estimate, bit 2: diff)
%          ntree     0
%          atree     0
%          labels    1 (bit 0: groundtruth, bit 1: estimate)


[step,rest] = get_step(solns,varargin,1);

[solns,step]=repmats(solns,step);

%firstfig = gcf;

%todo:direction

[printable,labels,cameras,direction,crange,object,geometry,doralign,ntree,atree,titles,type,layout,rest] = ...
  process_options(rest, 'printable', 0, ...
                        'labels', 1, ...
                        'cameras', 3, ...
                        'direction', 3, ...
                        'crange', 0, ...
                        'object', 3, ...
                        'geometry', 0, ...
                        'align', 1, ...
                        'ntree', 0, ...
                        'atree', 0, ...
                        'titles', [], ...
                        'belief', 'posterior', ...
                        'layout', []);

if printable
  fontsize=16;
  fontname='times';
else
  fontsize=10;
  fontname='helvetica';
end

if numel(solns)>1
  clf
  if isempty(layout) || all(layout==0)
    ncols = ceil(sqrt(numel(solns)));
    nrows = ceil(numel(solns)/ncols);
    layout = [nrows ncols];
  elseif layout(1)==0
    layout(1) = ceil(numel(solns)/layout(2));
  elseif layout(2)==0
    layout(2) = ceil(numel(solns)/layout(1));
  end
end

for si=1:numel(solns)

  if numel(solns)>1
    subplot(layout(1),layout(2),si);
  end
%  figure(firstfig+si-1);
  soln=solns(si);

  if isnumeric(step)
    tc = step(si);
    t = real(tc);
  else 
    t = nsteps(soln);
  end

  ex = get(soln, 'experiment');
  time = ex.evidence.time;

  % Plot the testbed
  tb = testbed(soln);
  plot(testbed(soln), ...
       'printable', printable, 'geometry', geometry, ...
       'cameras', bitand(cameras, 1), 'labels', bitand(labels, 1), ...
       'direction', bitand(direction, 1), rest{:});
  if ~printable && isnumeric(step) && t<length(time)
    title(sprintf('t = %.2fs, step=%d', time(t), t));
  end
  hold on;

  % Get some common statistics
  if doralign && isnumeric(step)
    %  if isfield(soln, 'transform')
    %  transform=soln.transform;
    %  else
    transform=ralign(soln);
  else
    transform=struct('R', eye(2), 'c', 1, 't', zeros(2,1));
  end
  R=transform.R;
  tt=transform.t;
  c=transform.c;

  dat = data(soln);
  ev = evidence(soln);

  if isnumeric(step)
    [xt,Pt,xtl,Ptl] = legacy(soln,tc,type);
  else
    [xt,Pt,xtl,Ptl] = encode_legacy(model(soln),step,10000);
  end
 
  if numel(Pt)==0 % plot w/o covariance - fake a very small one
    Pt = eye(size(xt,1))*1e-6;
  end

  oldmodel = legacy(model(soln));

  % Plot the actual and estimated trajectory
  %if any(options=='p')
  %  h=plot(etraj(1,:),etraj(2,:),'g-');
  %  set(h, 'linewidth', 4);
  %  for t=steps,
  %    h=text(prob.pos(1,t)+0.1,prob.pos(2,t)-0.1,num2str(t));
  %    set(h,'backgroundcolor',[1 1 1]);
  %  end
  %end

  % Plot the estimated camera centers
  nstdevs=-norminv(0.05/2); % 95% confidence interval in a normal distrib.
  if bitand(cameras, 2)
    for k=1:ncams(testbed(soln)),
      calib = tb.calib(k);
      ci = [1:2 oldmodel.ci(k,:)];
      if Pt(ci(3),ci(3))<9000, %hack hack hack
        center=approximateAngleDistribution(xt(ci),full(Pt(ci,ci)),...
          @fnUvCameraCenter, eye(2)*1e-6, oldmodel.param(k));
        for j=1:length(center.p),
          center.mus(:,j)=c*R*center.mus(:,j)+tt;
          center.covs(:,:,j)=c^2*R*center.covs(:,:,j)*R';
        end

        if ev.visible(k,t) && isnumeric(step)
          plot_opts = {'m'};
          plot(calib.pos(1),calib.pos(2),'mx');
        else
          plot_opts = {'b'};
        end
        [mu, sigma]=mixture2gaussian(center);
        dist=min(norm(mu(1:2)-calib.pos(1:2))/crange,1);
        mycol = [dist 0.6*(1-dist) 0];
        plot_opts = {};

        % Plot the position confidence bound
        h=plotMogError(center, 0.95, plot_opts);
        set(h, 'linewidth', 2, 'color', mycol);

        % Plot the pan confidence bound
        if bitand(direction,2)
          ci = oldmodel.ci(k,oldmodel.param(k).pani-2);
          meanT=pi-(xt(ci)+atan2(transform.R(1,2),transform.R(1,1)));
          sigmaT=sqrt(Pt(ci,ci));
          h=quiver([mu(1);mu(1)],[mu(2);mu(2)], ...
            [cos(meanT+nstdevs*sigmaT);cos(meanT-nstdevs*sigmaT)]/2, ...
            [sin(meanT+nstdevs*sigmaT);sin(meanT-nstdevs*sigmaT)]/2,0);
          set(h,'maxheadsize',3);
          set(h,'showarrowhead','off');
          set(h,'showarrowhead','on');
          h=draw_arc(meanT-nstdevs*sigmaT,meanT+nstdevs*sigmaT,mu(1:2),1/4);
        end
        
        if bitand(cameras,4)
          h=quiver(calib.pos(1),calib.pos(2),mu(1)-calib.pos(1),mu(2)-calib.pos(2),0);
          set(h,'maxheadsize',3);
          set(h,'showarrowhead','off');
          set(h,'showarrowhead','on');
          set(h,'color',mycol);
        end
        
        if bitand(labels,2)
%           if length(tb.cameras)>0 && ~printable
%             label=tb.cameras{k};
%           else
            label=num2str(k);
%          end
          h=text(mu(1)-cos(meanT)*0.3, mu(2)-sin(meanT)*0.3,label);
          set(h,'horizontalalignment', 'center');
          set(h,'fontname', fontname);
          set(h,'fontsize',fontsize);
          set(h,'color','k');
%          set(h,'color',plot_opts{:});
        end
      end
    end
  end


  % Plot the object location
  if bitand(object, 1) && isa(dat, 'simulated_data')
    plot(dat.trajectory(1,t), dat.trajectory(2,t), 'k+');
  end

  if bitand(object, 2)
    h=plotcov2(c*R*xt(1:2)+tt, c^2*R*full(Pt(1:2,1:2))*R', ...
      'conf', 0.95, 'plot-axes', 0);
    set(h, 'linewidth', 2);
    if crange==0
      set(h,'color','r'), 
    end
  end

  if bitand(object, 4) && ~isempty(xtl) % plot the estimates from all the nodes
    for k=1:ncams(testbed(soln)),
      plotcov2(c*R*xtl{k}+tt, c^2*R*Ptl{k}*R', ...
        'conf', 0.95, 'plot-opts', {'r'}, 'plot-axes', 0);
    end
  end

  if isdistributed(soln)

    if ntree || atree
      s = get_stats(soln);
    end

    % Plot the network junction tree
    %ttree = get(soln,'ttree');
    %tree = get(soln,'tree');
    %ex = get(soln,'experiment');
    %time = ex.evidence.time;

    if ntree 
      s2=get(soln);
      if isfield(s,'tree') && length(s.tree)>0
        % Find the latest entry for this time step
        i = find(s.tree.step<=t,1,'last');
        parents = s.tree.parents(i,:);
      elseif length(s2.tree)>0
        parents = s2.tree(end,:);
      else
        parents = [];
      end
      if ~isempty(parents)
        if sum(parents==0)>1
          h=plot_dtree(tb, parents, 'color', 'r');
        else
          h=plot_dtree(tb, parents, 'color', [0 0.6 0]);
        end
        if printable
          set(h,'linewidth',2);
        end
      end
    end

    if atree && length(s.atree)>0
      % Find the latest entry  for this time step
      i = find(s.atree.step<=t,1,'last');
      parents = s.atree.parents(i,:);
      if sum(parents==0)>1
        plot_dtree(tb, parents, 'color', 'r');
      else
        plot_dtree(tb, parents, 'color', [0 0.6 0]);
      end
    end
  end

  hold off;
  if length(titles)>0
    title(titles{si});
  end
  drawnow;

end

if ~printable && isnumeric(step) && t<length(time) && length(titles)>0
 annotation(...
  1,'textbox',...
  'Position',[0 0.9 1 0.1],...
  'LineStyle','none',...
  'FitHeightToText','off',...
  'HorizontalAlignment','center',...
  'String',{sprintf('t = %.2fs, step=%d', time(t), t)},...
  'VerticalAlignment','middle');
end

%figure(firstfig);
