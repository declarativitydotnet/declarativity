function part=random_partition(nodes, locs, count, nbrs)
% random_partition(nodes, locs, count, connectivity)
% generates random lines in the plane and recursively splits the
% partitions, up to the given count

part{1} = {nodes};

if ~exist('nbrs', 'var')
  nbrs = ones(max(nodes));
else
  assert(isconnected(nbrs,nodes));
end

i = 2;
while i<=count
  prev = part{i-1};
  % determine the partition to split
  for j=1:length(prev)
    nnodes(j) = length(prev{j});
  end
  
  [maxnodes,k] = max(nnodes);
  prevk = prev{k};
  
  mloc  = mean(locs(:,prevk),2);
  mdiff = locs(:,prevk) - mloc*ones(1,length(prevk));
  mloc = mloc(1:2,:);
  mdiff = mdiff(1:2,:);
  radius = sqrt(max(sum(mdiff.^2,1)));
  
  %generate two points in the unit disk
  while 1
    p = 2*rand(2)-1;
    if max(sum(p.^2,1))<=1
      break;
    end
  end
  
  % construct the equation of the line
  p = radius*p+mloc*ones(1,2);
  a = [-p(2,1)+p(2,2) p(1,1)-p(1,2)];
  b = a*p(:,1);
  
  left = prevk(find(a*locs(1:2,prevk)<b));
  right = setdiff(prevk,left);
  
  % check the connectivity of the resulting components
  if min(length(left),length(right))<1/4*(length(prevk))
    disp('too uneven')
    
  elseif ~(isconnected(nbrs, left) && isconnected(nbrs, right))
    disp('not connected')
    
  else
    % split prevk into left & right
    part{i} = {prev{1:k-1} left right prev{k+1:end}};
    disp(sprintf('%s -> %s,%s', mat2str(prevk), mat2str(left), mat2str(right)'));
    i = i+1;
  end
  
  
end

function value = isconnected(a,nodes)
if nargin==2
  a = a(nodes,nodes);
end
a = a+eye(length(a));
a = a^length(a);
value = all(a(1,:)~=0);
