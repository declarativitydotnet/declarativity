function [rms, nValid]=compute_rms_error(soln,step,type);
% [rms, nValid]=rms_error(soln)
% computes the RMS error for the given solution. called by solution/rms_error
%

tb = testbed(soln);
ev = evidence(soln);

s = get(soln);
old_model = legacy(model(soln));
[xt,Pt]=legacy(soln,step,type,0);
if isempty(xt)
  warning(['Solution does not have a valid posterior at time ' num2str(step)]);
  rms = []; nValid = 0;
else
   s.transform=ralign(soln);

   [x2, P2, valid] = slatToCameraXy(xt, Pt, old_model, s.transform);
%   [x2, P2, valid] = slatToCameraXy(xt, Pt, old_model);

  %    valid = valid & (sum(ev.visible,2)>1);
  valid = (sum(ev.visible,2)>1);
  nValid=sum(valid);

  if nValid<ncams(tb)
    warning(['Only ' num2str(sum(valid)) ' out of ' num2str(ncams(tb)) ' estimates are valid.']);
  end

  rms=0;

  %dist = ones(1,max(valid));
  
  for j=find(valid)',
    rms = rms + sum(sqr(x2(old_model.ci(j,1:2))-tb.calib(j).pos(1:2)));
  end

  rms = sqrt(rms/nValid);
end
