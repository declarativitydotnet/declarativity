function plotmixture(s,n,varargin)
% plotmixture(s,n, ...)
% plots leading n components of the mixture s
% Optional arguments
% basis [d x 3] matrix

[basis,rest] = process_options(varargin,'basis',eye(3));

[sorted,ind] = sort(s.alpha,'descend');
held = ishold;

for j=1:n
  i = ind(j);
  % right-multiply the Gaussian by the matrix
  ctr = (s.mu{i}*basis)';
  sigma = basis'*s.sigma{i}*basis;
  if j>1
    hold on;
  end
  plotcov3(ctr,sigma,rest{:});
  ctrcell=num2cell(ctr);
  text(ctrcell{:},num2str(i),'fontsize',16);
end

if ~held
  hold off
end


