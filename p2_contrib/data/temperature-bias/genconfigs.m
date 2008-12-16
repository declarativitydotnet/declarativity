function genconfigs(pattern, nnodes)
probnames = findprobs(pattern);
for i=1:length(probnames)
  name = probnames{i};
  load(name);
  [prior, likelihoods] = load_model(name, nnodes);
  required = nodevars(nnodes);
  save_flat(name, prior, likelihoods, required);
  dms=save_decomposable(name, prior, likelihoods, required);
  save_vars(name, argumentsf(dms));
  save_links(a, name, nnodes);
end
