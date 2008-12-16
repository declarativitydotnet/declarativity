function [priors,likelihoods] = load_model(filename, n)
% function [factors,t] = load_flat(filename, n, u)
% returns a set of factors, whose product equals the temperature and bias model

load(filename);

import prl.*;
t = tempvars(n);
b = biasvars(n);
tlam = model.tlambda(1:n,1:n);
tvec = tlam * model.tmean(1:n);

np = 0;

[is, js, ws] = find(tlam);

% temperature priors
for k=1:length(is)
  i=is(k);
  j=js(k);
  w=ws(k);
  if i==j
    priors(np+1) = canonical_gaussian(vec(t(i)), w, tvec(i)); 
    np = np + 1;
  elseif i<j
    priors(np+1) = canonical_gaussian(t([i j]), [0 w; w 0], zeros(2,1)); 
    np = np + 1;
  end
end 

% bias priors
for i=1:n
  priors(np+1) = canonical_gaussian(moment_gaussian(vec(b(i)), 0, model.biasvar));
  np = np + 1;
end

global u;
z = u.new_vector_variable('tempobs', 1);

% observation likelihoods
for i=1:n
  obs_model = moment_gaussian(vec(z), 0, model.obsvar, [b(i) t(i)], [1 1]);
  likelihoods(i) = obs_model.as_canonical_gaussian.restrict(assignment(z, model.obs(i)));
end
