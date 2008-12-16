function value = cpc(model)
% function value = cpc(model)
% returns the maximum number of cameras per clique

value = zeros(size(model));

for i=1:numel(model)

  clusters = get(model(i), 'assumed_clusters');

  for j=1:length(clusters),
    value(i) = max(value(i),length(clusters{j}));
  end
  
end
