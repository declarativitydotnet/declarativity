function save_tempcsv(basename, model, membership, target)

nnodes = max(max(membership));
nvars = size(membership,1);

lambda = model.tlambda(1:nvars, 1:nvars);
eta = lambda*model.tmean(1:nvars);

f = fopen([basename '-cliquepot-' num2str(nnodes) '.cpp'], 'wt');

for i=1:nnodes,
  % Save the diagonal entry
  fprintf(f, ['factors[%d].push_back(canonical_gaussian(seq(t[%d]),' ...
              'mat(%.16g),vec(%.16g)));\n'], i, i, lambda(i,i), eta(i));
  
  % Save the off-diagonal entries
  [is, js] = find(triu(membership==i, 1));
  w = lambda(sub2ind(size(lambda), is, js));
  for k=1:length(is) 
    fprintf(f, ['factors[%d].push_back(canonical_gaussian(seq(t[%d],t[%d]),' ...
                'mat(0,%.16g,%.16g,0), vec(0,0)));\n'], ...
            i, is(k), js(k), w(k), w(k));
  end
end

fclose(f);