function plotGaussian2D(mu1, sigma1, index, p1, w)
if ~exist('index', 'var')
    index=[1 2];
end
mu=mu1(index);
sigma=sigma1(index,index);
if exist('p1', 'var')
    p=p1(index,:);
 if ~exist('w','var')
    w=ones(1,size(p,2));
    maxw=5;
 else
    maxw = max(w);
 end
end

std=max(sqrt(diag(sigma)));
range=-2*std:std/50:2*std;
[u,v]=meshgrid(mu(1)+range, mu(2)+range);

puv=u;
puv(:) = mvnpdf([u(:) v(:)], mu', sigma);
[c,h]=contour(u,v,puv);
clabel(c,h);

if exist('p1', 'var')
    hold on;
    for i=1:10,
        wi=find((w>(i-1)*maxw/10) & (w<=i*maxw/10));
        h=plot(p(1,wi), p(2,wi), '.');
        set(h,'markersize',i*5);
    end
    hold off;
end

