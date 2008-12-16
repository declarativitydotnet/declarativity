function dms = save_decomposable(filebase, prior, likelihoods, required)
% save_decomposable(filename, prior, likelihoods, required)

assert(isempty(likelihoods) || length(likelihoods) == length(required));

import prl.*;

filename = [filebase '-' num2str(length(required)) '.db'];
dm = decomposable;
dm.multiply_in(prior);

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

% For any nodes that have node been covered yet, add the marginal 
% over the missing required variables
for j=1:n
  %if ~required(j).subset_of(dms(j).arguments)
    %remaining = required(j) - dms(j).arguments;
    remaining = required(j);
    dms(j) = dms(j) * dm.marginal(remaining).as_fragment_gaussian;
  %end
end

for j=1:n
  s(j).node_id = uint32(j);
  if ~isempty(likelihoods)
    dms(j) = dms(j) * likelihoods(j);
  end
  s(j).factor = dms(j);
  %s(j).vars = s(j).factor.arguments;
end

% multiplying in the likelihoods does not extend the arguments 
savesql(filename, 'decomposable', s);
