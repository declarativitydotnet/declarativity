function genconfigs(pattern, nnodes, varargin)

if nargin>1
  a = loadlinks(nnodes, varargin{:});
end
      
probnames = findprobs(pattern);
for i=1:length(probnames)
  name = probnames{i};
  load(name);
  save_isingcpp(model, name);
  if nargin>1
    a = diag(ones(1,nnodes-1),1); a = a+a';
    save_isingcsv(model, name);
    save_gridvars(model.m, model.n, 'F,2', name, nnodes);
    save_links(a, name, nnodes);
  end
end
