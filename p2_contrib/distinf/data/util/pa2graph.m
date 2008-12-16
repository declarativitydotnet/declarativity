function adj=pa2graph(parents)

adj = sparse(zeros(length(parents)));
for i=1:length(parents)
  pa = parents(i);
  if pa>0
    adj(i,pa) = 1;
    adj(pa,i) = 1;
  end
end

