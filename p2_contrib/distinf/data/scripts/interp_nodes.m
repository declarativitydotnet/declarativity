function yt = interp_nodes(table, t)
% table = (node, value, time)
table = sortrows(table,3);
nnodes = max(table(:,1));
yt = zeros(nnodes,length(t));
for i=1:nnodes
  j = find(table(:,1)==i);
  yt(i,:) = uniqueinterp(table(j,3)', table(j,2)', t);
end

