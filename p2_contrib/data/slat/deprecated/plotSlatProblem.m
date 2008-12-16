function nFigures = plotSlatProblem(problem, options)
% nFigures = plotSlatProblem(problem, options, varargin)
%
% options: 'a' - plot axes
%          'c' - camera centers 
%          'd' - camera centers and orientations
%          'v' - camera visibility range
%          't' - trajectory of the person (if available)
%          'o' - camera observations
%          'f' - use the current figure, rather than figure 1
%          'g' - geometry
%          'i' - NO grid
%          'p' - optimize for publication (larger fonts etc.)
%          '1' - rotate by 90 degrees
% default: 'dvtga' (if not specified)
%

% First, parse options 
if ~exist('options', 'var')
  options = 'dvtga';
else
  options=lower(options);
end

if any(options=='p')
  fontsize=16;
  fontname='times';
else
  fontsize=10;
  fontname='helvetica';
end

if length(find(options == 'f'))==0
  figure(1);
end

% Plot the geometry
calib=problem.calib;
cla;
axis equal
axis(problem.range);
if any(options=='a')
  axis on;
else
  axis off;
end

if any(options=='1')
	view(90,90);
end

if any(options=='p')
  %Fix for a MATLAB feature/bug
  ytick=get(gca, 'ytick');
  set(gca,'xtickmode','manual');
  set(gca,'xtick', problem.range(1):(ytick(2)-ytick(1)):problem.range(2));
  grid off;
else
  if any(options=='i')
    grid off;
  else
    grid on;
  end
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
if any(options=='g') & isfield(problem, 'geometry')
  geom=problem.geometry;
  for i=1:length(geom),
    line(geom{i}(:,1), geom{i}(:,2), 'Color', 'k');
  end
end

% Plot the cameras
if isfield(problem, 'height')
  height=problem.height;
else
  height=1.8;
end

for i=1:problem.nCams,
  p=-calib(i).R'*calib(i).T;
  theta = pi-calib(i).pan;
  
  if length(find(options == 'd'))>0
    h=quiver(p(1),p(2),cos(theta)*3/2, sin(theta)*3/2,0,'k');
    set(h,'marker','x');
    set(h,'maxheadsize',2);
    set(h,'showarrowhead','off');
    set(h,'showarrowhead','on');
  elseif any(find(options=='c'))
    plot(p(1),p(2),'xk');
  end
  if any(options=='c') || any(options=='d')
    if mod(theta,2*pi)>pi/2 & mod(theta,2*pi)<3*pi/2
      theta2=theta-3/4*pi;
    else
      theta2=theta+3/4*pi;
    end
    if isfield(problem, 'cameraid')
      k=problem.cameraid(i);
    else
      k=i;
    end
    h=text(p(1)+cos(theta2)*0.3, p(2)+sin(theta2)*0.3, num2str(k)); %mat2str(p(1:2)));
    set(h,'horizontalalignment', 'center');
    set(h, 'fontname', fontname);
    set(h,'fontsize',fontsize);
  end
  
  if length(find(options == 'v'))>0
    plotVisibilityRange(calib(i), height, 'b', 'LineStyle', '-.', 'FaceAlpha', 0.2);
  end
end
  
% Plot the trajectory
if isfield(problem, 'pos') & length(find(options == 't'))
  steps = [1 problem.nSteps]; %[1 20:20:problem.nSteps];
  h=plot(problem.pos(1,:),problem.pos(2,:),'r.');
  %set(h, 'MarkerSize', 10);
  for t=steps,
    h=text(problem.pos(1,t),problem.pos(2,t)+0.05,['\it{t} = ' num2str(t)]);
    %set(h,'backgroundcolor',[1 1 1]);
    set(h, 'fontsize', fontsize);
    set(h, 'fontname', fontname);
    set(h, 'verticalalignment', 'baseline');
  end
  %ppos=invertProjection(problem.obs(:,:,3),problem.calib(3),0,0,[0 0 1 -1.8]);
  %plot(ppos(1,:),ppos(2,:),'m-');
end

hold off;

% Plot the observations
if length(find(options == 'o'))>0
  h = gcf;
  figure(2);
  %if length(ecalib)>0 & isfield(problem, 'pos')
  %  plotSlatObservations(problem, ecalib(k));
  %else
    plotSlatObservations(problem);
  %end
  figure(h);
end

