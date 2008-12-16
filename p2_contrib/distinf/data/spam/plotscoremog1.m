function plotscoremog(s, cluster,varargin)
% Plots the expected score for a Mixture of Gaussian model

k = length(s.mu);
ctr = vertcat(s.mu{:});

for i=1:length(varargin)
  x = varargin{i};
  p = pmog(x, s);

  [stuff, ind] = max(p,[],2);
  ii = find(ind==cluster);
  p = p(ii,:);
  x = x(ii,:);
  
  p = p ./ repmat(sum(p,2),1,k);
  score=x*ctr'; % n x k matrix
  escore = sum(score.*p,2);
  ii = find(~isnan(escore));
  escore=escore(ii,:);
  args{1,i} = sort(escore);
  args{2,i} = (1:length(escore))/length(escore);
  count(i) = length(ii);
end

semilogx(args{:});
count