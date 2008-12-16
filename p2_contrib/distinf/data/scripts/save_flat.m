function dms = save_flat(filebase, prior, likelihoods, required)
% save_flat(filename, prior, likelihoods, required)

assert(isempty(likelihoods) || length(likelihoods) == length(required));

import prl.*;

filename = [filebase '-' num2str(length(required)) '.db'];

n = length(required);

pot = java_array('prl.factor',n);
pot(1:n) = constant_factor(1);

for i=1:length(required)
  assigned(i) = domain(required(i));
end

% Add each factor to the node that greedily minimizes the cluster size
% We could speed this up with a clique index
for i=1:length(prior)
  f = prior(i);
  % compute the intersection size with each required node and select one
  % with largest maximum intersection size and smallest overall size
  bestu = inf;
  for j=1:n
    usize = assigned(j).union_size(f.arguments);
    if usize < bestu && (j ~= 5 || f.arguments.subset_of(assigned(5)))
      bestu = usize;
      best = j;
    end
  end
  pot(best) = pot(best) * f;
  assigned(best).insert(f.arguments); 
end

% Mutliply in the observation likelihoods (if any)
for j=1:n
  s(j).node_id = uint32(j);
  if isempty(likelihoods)
    s(j).factor = pot(j);
  else
    s(j).factor = pot(j) * likelihoods(j);
  end
  %s(j).vars = s(j).factor.arguments;
end

savesql(filename, 'flat', s);
