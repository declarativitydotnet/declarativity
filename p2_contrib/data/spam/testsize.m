for i=1:4
  [reject{i},accept{i}]=loaddata(sprintf('20070301_%02d',i*6),100);
end

for i=1:4
  meanr(i) = mean(sum(reject{i},2));
  medianr(i) = median(sum(reject{i},2));
  meana(i) = mean(sum(accept{i},2));
  mediana(i) = median(sum(accept{i},2));
end
