function save_vars(filename, factors)

ids = [];
names = {};

for i = 1:length(factors)
  vars = factors(i).arguments; 
  ids = [ids; uint32(i*ones(vars.size,1))];
  names = [names varnames(vars)]; 
end

s = struct('node_id', num2cell(ids), 'name', java2cell(names));
savesql(filename, 'variables', s);
