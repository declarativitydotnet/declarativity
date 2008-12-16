function experiments = noise_sensitivity(ex, sigmas, n)
% function noise_sensitivity(ex, dt, n)
% takes in a single experiment and returns a |sigmas| x n array of
% experiments, in which we vary the observation noise according to
% sigmas for n times

s = get(ex);
oe  = s.evidence.algorithm;
dat = s.evidence.data;

for i=1:length(sigmas),
  oe = set(oe, 'sigma_u', sigmas(i), 'sigma_v', sigmas(i));
  newmodel = set(s.model, 'obs_var', sigmas(i)^2);
  for j=1:n,
    ev = run(oe, dat);
    experiments(i,j) = experiment(ev, s.algorithm, newmodel, s.scenario);
  end
end
