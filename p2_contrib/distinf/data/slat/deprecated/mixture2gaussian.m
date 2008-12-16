function [mu,sigma]=mixture2gaussian(p, mus, sigmas, domain)

if ~exist('sigmas', 'var'),
  if exist('mus', 'var') 
    domain=mus;
  end
  mog=p;
  p=mog.p;
  mus=mog.mus;
  sigmas=mog.covs;
end

if iscell(mus)
  for i=1:length(mus)
    mu(:,i) = mus{i};
    sigma(:,:,i) = sigmas{i};
  end
  mus = mu;
  sigmas=sigma;
end

%if any(p>0.99)
%  warning('Most of the weight is in a single mixture component.');
%end

[d,stuff]=size(mus);

if exist('domain','var')
  mu=zeros(d+1,1);
  sigma=zeros(d+1,d+1);
  
  for i=1:length(p),
    mu1=[mus(:,i);domain(i)];
    mu=mu+p(i)*mu1;
    sigma=sigma+p(i)*([sigmas(:,:,i) zeros(d,1);zeros(1,d+1)] + mu1*mu1');
  end

else

  mu=zeros(d,1);
  sigma=zeros(d,d);
  
  for i=1:length(p),
    mu=mu+p(i)*mus(:,i);
    sigma=sigma+p(i)*(sigmas(:,:,i)+mus(:,i)*mus(:,i)');
  end
end

sigma=sigma-mu*mu';
sigma=(sigma+sigma')/2;
