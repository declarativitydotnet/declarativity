function save_tempcsv(basename, model, membership, target)

nnodes = max(max(membership));
nvars = size(membership,1);

lambda = model.tlambda(1:nvars, 1:nvars);
eta = lambda*model.tmean(1:nvars);

f = fopen([basename '-cliquepot-' num2str(nnodes) '.csv'], 'wt');

% Store the prior potentials
for i=1:nnodes,
  % Save the diagonal entry
  fprintf(f, ['%d,-%d,%.16g,%.16g\n'], i, i, lambda(i,i), eta(i));
  
  % Save the off-diagonal entries
  [is, js] = find(triu(membership==i, 1));
  w = lambda(sub2ind(size(lambda), is, js));
  if ~isempty(is)
    fprintf(f, ['%d,-%d_-%d,0_%.16g_%.16g_0,0_0\n'], ...
            [repmat(i, size(is)) is js w w]');
  end
end

% Save the bias prior
fprintf(f, ['%d,%d,%.16g,0\n'], ...
        [1:nnodes; 1:nnodes; repmat(1/model.biasvar, 1, nnodes)]);
        
% Save the observation likelihoods
% true bias, observation, plug into a canonical gaussian model
t = global_variable('t', 1);
b = global_variable('b', 1);
z = global_variable('z', 1);
obs_mg = moment_gaussian(z, 0, model.obsvar, [t b], [1 1]);
obs_cg = canonical_gaussian(obs_mg);

for i=1:nnodes,
  obs_likelihood = restrict(obs_cg, z, model.obs(i));
  fprintf(f, ['%d,-%d_%d,'], i, i, i);
  fprintf(f, '%.16g_', obs_likelihood.lambda);  fprintf(f, ',');
  fprintf(f, '%.16g_', obs_likelihood.eta); fprintf(f, '\n');
end

fprintf(f,'Done,0,0,0\n');

fclose(f);
