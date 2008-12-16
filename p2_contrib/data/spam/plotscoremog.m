function escore=plotscoremog(s, varargin)
% Plots the expected score for a Mixture of Gaussian model

k = length(s.mu);
ctr = vertcat(s.mu{:});

for i=1:length(varargin)
  x = varargin{i};
  p = pmog(x, s);
  p = p ./ repmat(sum(p,2),1,k);
  score=x*ctr'; % n x k matrix
  escore = sum(score.*p,2);
  ii = find(~isnan(escore));
  escore=escore(ii,:);
  args{1,i} = sort(escore);
  args{2,i} = (1:length(escore))/length(escore);
end

semilogx(args{:});
