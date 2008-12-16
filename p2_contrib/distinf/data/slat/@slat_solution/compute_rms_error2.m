function [rms, nValid]=compute_rms_error2(soln,step,type);
% [rms, nValid]=rms_error(soln)
% computes the RMS error for the given solution. called by solution/rms_error
%

tb = testbed(soln);
ev = evidence(soln);

bel = get_belief(soln,step,type);

if isempty(bel)
  warning(['Solution does not have a valid posterior at time ' num2str(step)]);
  rms = []; nValid = 0;
else
  bel = to_absolute(model(soln), to_decomposable_model(bel));
  %s.transform=ralign(soln);

%  [x2, P2, valid] = slatToCameraXy(xt, Pt, old_model, s.transform);

  %    valid = valid & (sum(ev.visible,2)>1);
  valid = (sum(ev.visible,2)>1);
  nValid=sum(valid);

  if nValid<ncams(tb)
    warning(['Only ' num2str(sum(valid)) ' out of ' num2str(ncams(tb)) ' estimates are valid.']);
  end

  rms=0;
  vars = centervars(model(soln));
  %dist = ones(1,max(valid));
  
  for j=find(valid)',
    rms = rms + sum(sqr(mean(bel,vars(j))-tb.calib(j).pos(1:2)));
  end

  rms = sqrt(rms/nValid);
end
