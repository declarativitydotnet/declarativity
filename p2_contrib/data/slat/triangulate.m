function [cliques,tg]=triangulate(graph)
%triangulates a Markov graph using the edge fill heuristic
assert(size(graph,1)==size(graph,2));
assert(all(all(graph==graph')));
n = size(graph,1);
% get rid of the nodes w/o any neighbors
rest = setdiff(1:n,find(sum(graph,2)==0));

tg = graph | eye(n); % needed for fill-in computation; also, make tg binary

cliques={};

for i=1:length(rest),
  fill = ones(1,n)*inf;
  nbrs = cell(1,n);
  for k=1:length(rest)
    j = rest(k);
    nbrs{j} = find(tg(:,j));
    nbrs{j} = intersect(nbrs{j}, rest);
    fill(j) = length(nbrs{j})^2-sum(sum(tg(nbrs{j}, nbrs{j})));
  end
  [minfill,j]=min(fill)
  clique = zeros(n);
  clique(nbrs{j},nbrs{j})=1;
  cliques=[cliques {nbrs{j}}];
  tg = tg | clique;
  rest = setdiff(rest, j);
end

% Now absorb subsuming cliques

keep = ones(1,length(cliques));

for i=1:length(cliques),
  for j=1:length(cliques),
    if i~=j && isempty(setdiff(cliques{i}, cliques{j}))
      % cliques{i} is a subset of cliques{j}
      keep(i) = 0;
      break;
    end
  end
end

cliques=cliques(find(keep));