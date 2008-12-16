function [hvertices,hedges]=plotgraph(a,loc,varargin)
% plotgraph(a,loc,varargin)
% options: fontsize, highlight, groups

N = length(a);

[fontsize,highlight,groups,edges,labels,marker] = ...
    process_options(varargin,'fontsize',8,'highlight',[],'groups',[],'edges',1,'labels',[],'marker','o');

if isempty(groups)
  if isempty(highlight)
    groups = {1:N};
  else
    highlight =highlight(1:N);
    groups = {find(~highlight) find(highlight)};
  end
end

colors = 'brgk';

dim = size(loc,1);
assert(dim==2 || dim==3);

% plot the vertices
args = cell(2,length(groups));
for i=1:length(groups)
  args{1,i} = loc(1,groups{i});
  args{2,i} = loc(2,groups{i});
  if dim==3
    args{3,i} = loc(3,groups{i});
  end
  args{dim+1,i} = [marker];% colors(i)];
end
if dim==2
  hvertices = plot(args{:});
else
  hvertices = plot3(args{:});
  xlabel('x');
  ylabel('y');
  zlabel('z');
end

if dim==2
  zval = zeros(1,N);
else
  zval = loc(3,:);
end

% plot the edges
if edges
  [i,j]=find(a);
  hedges=line([loc(1,i);loc(1,j)],[loc(2,i);loc(2,j)],[zval(i);zval(j)]);
  set(hedges,'color','k');
end

% plot the labels
if fontsize 
  for i=1:length(groups)
    color = get(hvertices(i),'color');
    for j=reshape(groups{i},1,[]);
      if isempty(labels)
        label = j;
      else
        label = labels(j);
      end
      h=text(loc(1,j),loc(2,j),zval(j),num2str(label),'fontsize',fontsize,'verticalalign','bottom','color',color);
    end
  end
end
