function h=plotclusters(a,varargin)

if isempty(varargin) || ischar(varargin{1})
  cluster = ones(size(a,1),1);
else
  cluster = varargin{1};
  varargin = varargin(2:end);
end

[projection,normalization] = ...
  process_options(varargin,'projection','pca','normalization',0);

switch projection
  case 'pca'
    [v,b] = pca(a,3,normalization);
  case 'first'
    b = normrows(a);
    v = zeros(size(a,2),3);
    v(1:3,1:3)=eye(3);
end
proj=b*v;
      
for i=1:max(cluster)
  c=find(cluster==i);
  cc(i) = length(c);
  args{1,i} = proj(c,1);
  args{2,i} = proj(c,2);
  args{3,i} = proj(c,3);
  args{4,i} = '.';
end

disp('Cluster counts')
disp(cc)

h=plot3(args{:});
