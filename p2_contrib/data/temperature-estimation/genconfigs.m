function genconfigs(pattern, nnodes)
load link_quality

probnames = findprobs(pattern);
for i=1:length(probnames)
  name = probnames{i};
  [prior, likelihoods] = load_model(name, nnodes);
  required = var2domain(tempvars(nnodes));
  save_flat(name, prior, likelihoods, required);
  dms=save_decomposable(name, prior, likelihoods, required);
  save_vars(name, argumentsf(dms));
  save_links(a, name, nnodes);
end
