function yt = sum_nodes(table, t)
if isempty(table)
  yt = [];
  return;
end
table = table(~isnan(table(:,end)),:);
table = sortrows(table,size(table,2));
nnodes = max(table(:,1));
yt = zeros(length(t),size(table,2)-2);
for i=1:nnodes
  j = find(table(:,1)==i);
  if length(j)>1
    unique = [1; find(diff(table(j,end)))+1];
    j=j(unique);
    yt = yt + interp1(table(j,end), table(j,2:end-1), reshape(t,[],1),'linear','extrap');
  elseif length(j)==1
    yt = yt + repmat(table(j,2:end-1), size(yt,1), 1);
  end
end

