function labels=kmeanslabels(cluster, y)
% assign the most frequent label to each cluster
for i=1:max(cluster)
  labels(i,1) = mode(y(cluster==i)); 
end
