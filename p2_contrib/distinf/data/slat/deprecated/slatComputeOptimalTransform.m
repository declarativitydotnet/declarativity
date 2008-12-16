function transform=slatComputeOptimalTransform(prob, model, soln, time_step)

if ~exist('time_step', 'var')
  time_step=prob.nSteps;
end

[x2, P2, valid] = ...
  slatToCameraXy(soln.xt(:,time_step), soln.Pt{time_step}, model);

X  = cell2mat({prob.calib.pos}); 
X  = X(1:2,find(valid));
for k=1:prob.nCams,
  Xe(:,k) = x2(model.ci(k,1:2));
end
Xe = Xe(:,find(valid));

[c, R, t]=ralign(Xe, X);
transform=struct('c', c, 'R', R, 't', t);
