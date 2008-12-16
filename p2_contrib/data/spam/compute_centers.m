function ctr=compute_centers(a,cluster)

ctr = zeros(max(cluster),size(a,2));
for i=1:max(cluster)
  j=find(cluster==i);
  ctr(i,:) = mean(a(j,:));
  ctr(i,:) = ctr(i,:) / sum(ctr(i,:));
end
