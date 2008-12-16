function [cluster,s,iters] = emmog(a,k,varargin)
% function emmog(a,k)
% Runs EM algorithm with spherical Gaussians
% Inputs: a: one datapoint per row, k: number of clusters
% Optional arguments: 'cov' and 'regul' (regularization)

niters = 100;

[n,d] = size(a);

[cov_type,regul,labels,threshold,split] = ...
  process_options(varargin, 'cov', 'diag', 'regul', 1e-6,'labels',[],'threshold',1e-8,'split',0);

% initialize the cluster centers to random data points
p = randperm(n);
sigma_all = mycov(a,cov_type,regul);
for i=1:k
  mu{i} = a(p(i),:);
  sigma{i} = sigma_all;
end
alpha = ones(1, k)/k;

pold = ones(n,k)/k;

if split
  nl = max(labels);
  for i=1:k
    trainind{i} = find(labels==mod(i-1,nl)+1);
    assert(~isempty(trainind{i}));
  end
end

for it=1:niters,
  % compute the probability of each point for each cluster
  % p(j,i) = prob. that datapoint j belongs to cluster i
  p = zeros(n, k);
  for i=1:k
    if split
      j = trainind{i};
      p(j,i) = mymvnpdf(a(j,:), mu{i}, sigma{i}, cov_type) * alpha(i);
    else
      p(:,i) = mymvnpdf(a, mu{i}, sigma{i}, cov_type) * alpha(i);
    end
  end
  p1 = p;
  p = p ./ repmat(sum(p, 2), 1, k);
  j = ~isnan(p(:,1));
  nans = sum(isnan(p(:,1)));
  if nans
    %warning(['Ignoring ' num2str(nans) ' entries']);
  end
  
  % compute the cluster means and covariances
  for i=1:k
    w = p(j,i)/sum(p(j,i));
    mu{i} = w'*a(j,:);
    ac = a(j,:) - repmat(mu{i},sum(j),1);
    switch cov_type
      case 'diag'
        sigma{i} = diag(sparse(w'*(ac.*ac))+regul);
      case 'full'
        sigma{i} = ac'*(repmat(w,1,d).*ac)+diag(ones(1,d)*regul);
    end
  end
  
  % compute the cluster probabilities
  alpha = mean(p(j,:), 1);
  dp = norm(p(j,:)-pold(j,:),'fro');
  loglik = sum(log(sum(p1(j,:),2)));
  disp(sprintf('%d\t%d\t%d', it, dp, loglik));
  pold = p;
  
  %ctr = vertcat(mu{:});
  %sum(ctr,2)
  
  if nargout>2
    iters(it).mu = mu;
    iters(it).sigma = sigma;
    iters(it).alpha = alpha;
    iters(it).p = p;
    if ~isempty(labels)
      iters(it).cp = classprob(p(j,:), labels(j));
    end
  end
  
  if dp<threshold, break, end
end

s.mu = mu;
s.sigma = sigma;
s.alpha = alpha;
s.p = p;
if ~isempty(labels)
  s.cp = classprob(p(j,:), labels(j));
end

[dummy, cluster] = max(p, [], 2);

function sigma=mycov(a, type, regul)
switch type
  case 'diag'
    sigma = diag(sparse(var(a)+regul));
  case 'full'
    sigma = cov(a)+diag(sparse(ones(1,size(a,2)))*regul);
  otherwise
    error('unknown type');
end

function p=mymvnpdf(a, mu, sigma, type)
switch type
  case 'diag'
    std = full(sqrt(diag(sigma)))';
    n = size(a,1);
    p = prod(normpdf(a, repmat(mu,n,1), repmat(std,n,1)),2);
  case 'full'
    p = mvnpdf(a, mu, sigma);
end

function cp=classprob(p, labels)
for i=1:max(labels)
  ind = labels==i;
  cp(i,:) = sum(p(ind,:),1) / length(labels);
end
