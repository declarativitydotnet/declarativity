function vars = assign_vars(adjacency)

N = size(adjacency, 1);

[is js val] = find(adjacency);

vars = num2cell(1:N);

for k=1:length(is) 
  i = is(k); j = js(k);
  if i<j 
    varsi = union(vars{i}, [i j]);
    varsj = union(vars{j}, [i j]);
    if length(varsi)<length(varsj) 
      vars{i} = varsi;
    else
      vars{j} = varsj;
    end
  end
end
    
