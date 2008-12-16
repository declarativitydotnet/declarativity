function graph = assumed_mn(model)

clusters = model.assumed_clusters;
graph = zeros(length(model.calib));

for i=1:length(clusters),
  ids = clusters{i};
  graph(ids,ids)=1;
end
