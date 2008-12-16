function [upper,lower]=compute_rms_bounds(soln,step,type);
% function [upper,lower]=compute_rms_bounds(soln,step,type);
% computes the RMS error for the given solution. called by
% solution/rms_bounds
%

tb = testbed(soln);
ev = evidence(soln);

s = get(soln);
old_model = legacy(model(soln));
bel = get_belief(soln,step,type,0);
if isempty(bel)
  warning(['Solution does not have a valid posterior at time ' num2str(step)]);
  upper = []; lower = [];
else
  valid = (sum(ev.visible,2)>1);
  nValid=sum(valid);

  if nValid<ncams(tb)
    warning(['Only ' num2str(sum(valid)) ' out of ' num2str(ncams(tb)) ' estimates are valid.']);
  end

  if iscell(bel)
    bel = [bel{:}];
  end
  if iscell(bel)
    bel = [bel{:}];
  end
  
  if isa(bel,'decomposable_model')
    bel = [bel.marginals];
  end
  s.transform=ralign(soln);
  
  mind = ones(1,ncams(tb))*inf;
  maxd = zeros(1,ncams(tb));
  
  locs=node_locations(tb); 
%  a=load('test');
  
  for i=1:length(bel)
    [xt,Pt,xtl,Ptl,cams] = encode_legacy(model(soln), bel(i));
%     myi = [];
%     for j=1:length(cams)
%       myi = [myi cams(j)*5:cams(j)*5+4];
%     end
    [x2, P2] = slatToCameraXy(xt, Pt, old_model, s.transform, cams);
    % WARNING: HACK (1:length(cams))
    dist = sum((x2(old_model.ci(1:length(cams),1:2)')-locs(1:2,cams)).^2,1);
%     assert(norm(a.x2(myi)-x2(5:end))<1e-4);
    maxd(cams) = max(maxd(cams),dist);
    mind(cams) = min(mind(cams),dist);
  end
  
  upper = sqrt(mean(maxd(valid)));
  lower = sqrt(mean(mind(valid)));
end
