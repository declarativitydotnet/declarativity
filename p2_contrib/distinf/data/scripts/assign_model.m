function [adjacency,vars] = assign_model(adjacency)

N = size(adjacency, 1);

[is js val] = find(adjacency);

vars = num2cell(1:N);

model = [];

for k=1:length(is) 
  i = is(k); j = js(k);
  if i<j 
    varsi = union(vars{i}, [i j]);
    varsj = union(vars{j}, [i j]);
    if length(varsi)<length(varsj) 
      vars{i} = varsi;
      adjacency(i,j) = i;
      adjacency(j,i) = i;
    else
      vars{j} = varsj;
      adjacency(i,j) = j;
      adjacency(j,i) = j;
    end
  end
end
    
adjacency = adjacency - diag(diag(adjacency));
adjacency = adjacency + diag(1:N);
