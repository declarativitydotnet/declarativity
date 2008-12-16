function a=converged(values, threshold)
a = ones(size(values));
for i=1:size(values,1)
  j=find(values(i,:)>=threshold,1,'last');
  a(i,1:j) = 0;
end
