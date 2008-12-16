function clique_set=cliques(model);

lvars = locvars(model);
pvars = posevars(model);

clusters = get(model, 'assumed_clusters');

for i=1:length(clusters)
  clique_set{i} = [lvars' reshape(pvars(:,clusters{i}),1,[])];
end

