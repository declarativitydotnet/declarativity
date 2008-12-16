function plotscore(ctr,varargin)
% function plotscore(ctr, dataset1, [dataset2, ] ...)
% ctr is [k x d] matrix of scores
for i=1:length(varargin)
  a = varargin{i};
  n = size(a,1);
  score = compute_score(ctr,a);
  args{1,i} = sort(score);
  args{2,i} = (1:n)/n;
end

semilogx(args{:});
%plot(args{:});
