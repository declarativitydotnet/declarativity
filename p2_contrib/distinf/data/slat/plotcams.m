function plotcams(prob, varargin)
% plot(testbed, ,,,)
%
% options: 'axis'       1
%          'visibility' 1
%          'cameras'    1
%          'direction'  1
%          'labels'     1
%          'geometry'   1
%          'rotate'     0
%          'printable'  0
%          'graph'      0

[plotaxis,visibility,cameras,camtype,direction,labels,geometry,rotate,printable,graph,wgraph] = ...
  process_options(varargin, 'axis', 1, ...
                            'visibility', 0, ...
                            'cameras', 1, ...
                            'camtype','point',...
                            'direction', 1, ...
                            'labels', 1, ...
                            'geometry', 1, ...
                            'rotate', 0, ...
                            'printable', 0, ...
                            'graph', [], ...
                            'wgraph',[]);

tb = prob;

if printable
  fontsize=16;
  fontname='times';
else
  fontsize=10;
  fontname='helvetica';
end

% Plot the geometry
calib=tb.calib;
cla;
axis equal
axis(tb.range);
if plotaxis
  axis on;
else
  axis off;
end

if rotate
	view(90,90);
end

if printable
  %Fix for a MATLAB feature/bug
  ytick=get(gca, 'ytick');
  set(gca,'xtickmode','manual');
  set(gca,'xtick', tb.range(1):(ytick(2)-ytick(1)):tb.range(2));
  grid off;
else
  %grid on;
end
xlabel('\it{x}');
ylabel('\it{y}');
title('');
handles=[gca get(gca, 'xlabel') get(gca, 'ylabel') get(gca, 'zlabel')];
for h=handles,
  set(h, 'fontname', fontname);
  set(h, 'fontsize', fontsize);
end
hold on;

if geometry && ~isempty(tb.geometry)
  geom=tb.geometry;
  for i=1:length(geom),
    line(geom{i}(:,1), geom{i}(:,2), 'Color', 'k');
  end
end

% Plot the cameras

for i=1:tb.ncams,
  p=-calib(i).R'*calib(i).T;
  theta = calib(i).pan+pi/2;
  
  if cameras && direction
    h=quiver(p(1),p(2),cos(theta)*3/2, sin(theta)*3/2,0,'k');
%    set(h,'marker',0);
    set(h,'maxheadsize',2);
    set(h,'showarrowhead','off');
    set(h,'showarrowhead','on');
  end
  if cameras
    switch camtype
      case 'point'
        h=plot(p(1),p(2),'xk');
        if printable
          set(h,'markersize',12);
        end
      case 'overhead'
        corners = [cos(theta) -sin(theta);sin(theta) cos(theta)]*diag([0.4 0.3]*0.6)*[-1 1 1 -1;-1 -1 1 1];
        fill(corners(2,:)+p(1),corners(1,:)+p(2),'b');
        h=rectangle('position',[p(1)-0.08 p(2)-0.08 0.16 0.16],'curvature', [1 1]);
        set(h, 'facecolor', [0.5 0.5 1]);
      case 'sidefacing'
        corners = [cos(theta) sin(theta);-sin(theta) cos(theta)]*diag([0.4 0.3]*0.6)*[-1 1 1 -1;-1 -1 1 1];
        fill(corners(2,:)+p(1),corners(1,:)+p(2),'b');
        corners = [cos(theta) sin(theta);-sin(theta) cos(theta)]*diag([0.4 0.3]*0.6)*[-0.5 0.5 0.75 -0.75;1 1 2 2];
        fill(corners(2,:)+p(1),corners(1,:)+p(2),'w');
      otherwise
        error('Wrong camera type')
    end
  end

  if labels
%     if mod(theta,2*pi)>pi/2 & mod(theta,2*pi)<3*pi/2
%       theta2=theta-3/4*pi;
%     else
      theta2=theta+3/4*pi;
%     end
%     if length(tb.cameras)>0 && ~printable
%       label=tb.cameras{i};
%     else
    label=num2str(i);
%     end
    if labels
      if cameras && strcmp(camtype,'overhead')
        dist = 0.4;
      else
        dist = 0.3;
      end
      h=text(p(1)+cos(theta2)*dist, p(2)+sin(theta2)*dist,label); %mat2str(p(1:2)));
      set(h,'horizontalalignment', 'center');
      set(h,'verticalalignment','middle');
      set(h, 'fontname', fontname);
      set(h,'fontsize',fontsize);
    end
  end
  
  if visibility
    plotvisibility(calib(i), prob.height, 'b', 'LineStyle', '-.', 'FaceAlpha', 0.2);
  end
end

loc = [tb.calib.pos];

if ~isempty(graph)
  % Plot the graph
  for i=1:min(tb.ncams,length(graph))
    for j=(i+1):min(tb.ncams,length(graph))
      if graph(i,j)
        h=line(loc(1,[i j]),loc(2,[i j]));
        if printable
          set(h,'linewidth',2);
        end
      end
    end
  end
end

if ~isempty(wgraph)
  range = max(max(loc(1,:))-min(loc(1,:)),max(loc(2,:))-min(loc(2,:)));
  motes = 1:tb.ncams;
  for i=1:min(length(motes),length(wgraph))
    for j=(i+1):min(length(motes),length(wgraph))
      if wgraph(i,j)>0
        delta = [loc(2,j)-loc(2,i);loc(1,i)-loc(1,j)];
        delta = delta/norm(delta)*range/300;
        h=fill([loc(1,[i j])+delta(1) loc(1,[j i])-delta(1)], ...
               [loc(2,[i j])+delta(2) loc(2,[j i])-delta(2)], 'b');
        set(h,'facealpha',wgraph(i,j),'linestyle','none');
      end
    end
  end
end
 

hold off;
