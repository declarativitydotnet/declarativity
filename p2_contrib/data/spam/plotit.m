function plotit(a,s,n,varargin)
[projection,normalization] = process_options(varargin,'projection','pca','normalization',0);

switch projection
  case 'pca'
    v=pca(a,3,normalization);
  case 'first'
    v=zeros(size(a,2),3);
    v(1:3,1:3)=eye(3);
end 
p=pmog(a,s);
[dummy,clusters]=max(p,[],2);
plotclusters(a,clusters,varargin{:})
hold on;
plotmixture(s,n,'basis',v,'surf-opts', ...
  {'EdgeAlpha', 0, 'FaceAlpha', 0.1, 'facecolor','g'});
hold off;


