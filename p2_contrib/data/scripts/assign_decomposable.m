function dms = assign_decomposable(dm, required)
% assign_decomposable(dm, model)
% saves the model as a decomposable fragment
assert(isa(dm, 'prl.decomposable'));

import prl.*;

marginals = dm.clique_marginals;
n = length(required);

dms = java_array('prl.factor',n);
dms(1:n) = fragment_gaussian;

% First add the clique, optimizing our objective
for i=1:length(marginals)
  f = marginals(i);
  % compute the intersection size with each required node and select one
  % with largest maximum intersection size and smallest overall size
  bestint = 0;
  besttot = inf;
  for j=1:n
    isize = required(j).intersection_size(f.arguments);
    tsize = dms(j).arguments.plus(f.arguments).size;
    if isize > bestint || (isize == bestint && tsize < besttot)
      bestint = isize;
      besttot = tsize;
      best = j;
    end
  end
  dms(best) = dms(best) * f.as_fragment_gaussian; % could be .insert(f)
end

% Ensure that each node has a cqlique marginal over the required variables
% (this is necessary for temperature-bias estimation)
for j=1:n
  % we must use as_fragment_gaussian; otherwise, the marginal will be 
  % treated as a likelihood
  dms(j) = dms(j) * dm.marginal(required(j)).as_fragment_gaussian;
%   if ~required(j).subset_of(dms(j).arguments)
%     remaining = required(j) - dms(j).arguments;
%     dms(j) = dms(j) * dm.marginal(remaining).as_fragment_gaussian;
%   end
end
