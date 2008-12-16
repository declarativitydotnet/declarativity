function result=istriangulated(tree, cliques)

n = size(tree,1);
assert(n == length(cliques));

var_cliques={};
offset = -min([cliques{:}])+1;

for i=1:length(cliques)
  vars = cliques{i}+offset;
  for j=1:length(vars)
    if length(var_cliques)<vars(j)
      var_cliques{vars(j)} = i;
    else
      var_cliques{vars(j)} = [var_cliques{vars(j)} i];
    end
  end
end

result=isjtree(var_cliques, tree);
