function transform=ralign(soln, time_step)
% computes the optimal rigid alignment 
% TODO: get rid of the scale in the computation

if ~exist('time_step', 'var')
  time_step=nsteps(evidence(soln));
end

s = get(soln);

old_model = legacy(model(soln));
tb = testbed(soln);
[xt,Pt]=legacy(soln, time_step);

[x2, P2, valid] = slatToCameraXy(xt, Pt, old_model);

valid = valid & (sum(s.experiment.evidence.visible,2)>1);


calib = tb.calib;
X  = cell2mat({calib.pos}); 
X  = X(1:2,find(valid));
for k=1:ncams(testbed(soln))
  Xe(:,k) = x2(old_model.ci(k,1:2));
end
Xe = Xe(:,find(valid));

[c, R, t]=ralign(Xe, X);
disp(['ralign computed scale ' num2str(c) ', setting to 1']);
transform=struct('c', c, 'R', R, 't', t);
