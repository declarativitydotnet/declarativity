function [cg,t,b]=joint(model, nvars)

lambda = model.tlambda(1:nvars, 1:nvars);
eta = lambda*model.tmean(1:nvars);

for i=1:nvars
  t(i) = global_variable(sprintf('t%d',i), 1);
  b(i) = global_variable(sprintf('b%d',i), 1);
end
z = global_variable('z', 1);

cg_t = canonical_gaussian(t, eta, lambda);
cg_b = canonical_gaussian(b, zeros(nvars,1), diag(ones(nvars,1)/model.biasvar));

obs_lik = canonical_gaussian;

for i=1:nvars
  obs_mg = moment_gaussian(z, 0, model.obsvar, [t(i) b(i)], [1 1]);
  obs_cg = canonical_gaussian(obs_mg);
  obs_lik = combine(obs_lik, restrict(obs_cg, z, model.obs(i)));
end

cg = combine(combine(cg_t, cg_b), obs_lik);

