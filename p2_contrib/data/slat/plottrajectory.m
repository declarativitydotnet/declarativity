function plottrajectory(prob, soln, varargin)
plotcams(prob, varargin{:});

% s = get_stats(soln);

% no rigid alignment for now
hold on;
plot(soln(2,:),soln(3,:),'r.');

% todo: plot the variance

steps = [1 size(soln,2)];
for t=steps
  h=text(soln(2,t), soln(3,t)+0.05,['\it{t} = ' num2str(t)]);
%  h=text(s.meanl(1,t),s.meanl(2,t)+0.05,['\it{t} = ' num2str(t)]);
%   set(h,'backgroundcolor',[1 1 1]);
%   set(h, 'fontsize', fontsize);
%   set(h, 'fontname', fontname);
%   set(h, 'verticalalignment', 'baseline');
end

hold off;
