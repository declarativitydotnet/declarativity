function stats = compute_stats(sol)

stats = compute_stats(sol.solution);

return;

s = get(sol);
model = s.experiment.model;

vars = nodevars(model);
lvars = locvars(model);

for t=1:nsteps(sol)
  t
  belief = posterior(sol,t);
  
  if ~iscell(belief)
    margl = marginal(belief,lvars);
    meanl(:,t) = margl.const;
    varl(:,t)  = diag(margl.cov);
  end
  
  for i=1:size(vars,2)
    if iscell(belief)
      marg  = marginal(belief{i},vars(:,i));
      margl = marginal(belief{i},lvars);
      meanl(:,t,i) = margl.const;
      varl(:,t,i) = diag(margl.cov);
    else
      marg = marginal(belief, vars(:,i));
    end
    mean(:,t,i) = marg.const;
    var(:,t,i) = diag(marg.cov);
  end
end

stats.mean  = mean;
stats.var   = var;
stats.meanl = meanl;
stats.varl  = varl;
