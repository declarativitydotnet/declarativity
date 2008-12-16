function h=plotMixtureGaussian2D(p, mu1, sigma1, index, values)
%plotMixtureGaussian2D(p, mu1, sigma1, index)
if ~exist('index', 'var')
    index=[1 2];
end

if iscell(mu1)
  for i=1:length(mu1)
    mu(:,i) = mu1{i};
    sigma(:,:,i) = sigma1{i};
  end
  mu=mu(index,:);
  sigma=sigma(index,index,:);
else
  mu=mu1(index,:);
  sigma=sigma1(index,index,:);
end


nSteps=100;

std=0;
xmin=min(mu(1,:)'-2*sqrt(squeeze(sigma(1,1,:))));
xmax=max(mu(1,:)'+2*sqrt(squeeze(sigma(1,1,:))));
ymin=min(mu(2,:)'-2*sqrt(squeeze(sigma(2,2,:))));
ymax=max(mu(2,:)'+2*sqrt(squeeze(sigma(2,2,:))));

[u,v]=meshgrid(xmin:(xmax-xmin)/nSteps:xmax, ymin:(ymax-ymin)/nSteps:ymax);
puv=zeros([size(u)]);

for i=1:length(p),
  if p(i)>1e-10
    puv(:) = puv(:)+p(i)*mvnpdf([u(:) v(:)], mu(:,i)', sigma(:,:,i));
  end
end

%surf(u,v,puv);
%shading flat
[c,h]=contour(u,v,puv);

if exist('values','var')
  hold on;
  for i=1:floor(length(p)/20):length(p),
    text(mu(1,i),mu(2,i),num2str(values(i)));
  end
  hold off
end
