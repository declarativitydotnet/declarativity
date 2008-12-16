function h=plotscore1(ctr,idx,varargin)
% function plotscore(ctr, dataset1, [dataset2, ] ...)
% ctr is [k x d] matrix of scores
for i=1:length(varargin)
  a = varargin{i};
  n = size(a,1);
  [score,cluster] = compute_score(ctr,a);
  score = score(cluster==idx);
  args{1,i} = sort(score);
  args{2,i} = (1:length(score))/length(score);
  count(i) = length(score);
end
disp(count);
h=semilogx(args{:});
%plot(args{:});
