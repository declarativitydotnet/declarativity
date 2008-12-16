function valid=valid_nodes(sol, step, type)
assert(isscalar(sol));

if nargin<3
  type = 'posterior';
end

bel = get_belief(sol, step, type);

if iscell(bel)
  bel = [bel{:}];
  if isa(bel(1),'decomposable_model')
    bel = [bel.marginals];
  end
end

vars = nodevars(model(sol));
vars = vars(1,:);
n = length(vars);
valid=zeros(1,n);

%%% This is super-uglyy
for i=1:numel(bel)
  p = bel(i);
  pvars = intersect(args(p),vars);
  var = variance(p,pvars);
  var = var(1:length(var)/length(pvars):length(var));
  j = find(vars,pvars);
  valid(j) = valid(j) | (var'<9e3);
end

  


