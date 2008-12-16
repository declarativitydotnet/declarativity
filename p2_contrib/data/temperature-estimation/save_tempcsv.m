function save_tempcsv(basename, model, membership, target)

nnodes = max(max(membership));
nvars = size(membership,1);

lambda = model.tlambda(1:nvars, 1:nvars);
eta = lambda*model.tmean(1:nvars);

% Store the clique potentials
f = fopen([basename '-cliquepot-' num2str(nnodes) '.csv'], 'wt');

for i=1:nnodes,
  % Save the diagonal entry
  fprintf(f, ['%d,%d,%.16g,%.16g\n'], i, i, lambda(i,i), eta(i));
  
  % Save the off-diagonal entries
  [is, js] = find(triu(membership==i, 1));
  w = lambda(sub2ind(size(lambda), is, js));
  if ~isempty(is)
    fprintf(f, ['%d,%d_%d,0_%.16g_%.16g_0,0_0\n'], [repmat(i, size(is)) is js w w]');
  end
end

fprintf(f,'Done,0,0,0\n');

fclose(f);
