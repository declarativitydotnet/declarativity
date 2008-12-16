function save_vars(filename, sets)

n = length(sets);
s = struct('node_id', cell(1, n), 'variables', cell(1, n));

for i = 1:n
  s(i).node_id = uint32(i);
  s(i).variables = sets(i); 
end

savesql([filename '-' num2str(n) '.db'], 'variables', s);
