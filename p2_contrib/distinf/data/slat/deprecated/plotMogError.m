function h=plotMogError(mog, confidence, plot_opts)

if length(mog.p)==1,
  h=plotcov2(mog.mus,mog.covs,'conf', confidence, 'plot-opts', plot_opts, ...
    'plot-axes', 0);
else
  resolution=20;

  minx=min(mog.mus(1,:)'-3*sqrt(squeeze(mog.covs(1,1,:))));
  miny=min(mog.mus(2,:)'-3*sqrt(squeeze(mog.covs(2,2,:))));
  maxx=max(mog.mus(1,:)'+3*sqrt(squeeze(mog.covs(1,1,:))));
  maxy=max(mog.mus(2,:)'+3*sqrt(squeeze(mog.covs(2,2,:))));
  stepx=(maxx-minx)/resolution;
  stepy=(maxy-miny)/resolution;
  
  [x,y]=meshgrid(minx:stepx:maxx, miny:stepy:maxy);
  pxy=zeros(size(x,1),size(y,1));

  for i=1:length(mog.p),
    pxy(:)=pxy(:)+mog.p(i)*mvnpdf([x(:) y(:)], mog.mus(:,i)', mog.covs(:,:,i));
  end
  pxydesc = sort(pxy(:),1,'descend');
  pxysum  = cumsum(pxydesc/sum(pxydesc));
  indices = find(pxysum>=confidence); % there ought to always be at least one if confidence<=1
  level = pxydesc(indices(1));
  if ~isempty(plot_opts)
    [c,h]=contour(x,y,pxy,[level level],plot_opts{1});
  else
    [c,h]=contour(x,y,pxy,[level level]);
  end
end
  
  
    