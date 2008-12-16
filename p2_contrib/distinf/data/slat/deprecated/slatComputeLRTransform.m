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

t = -mean(Xe,2);

y  = Xe(2,:)';
Xe = Xe(1,:)';
Xe = [Xe ones(size(Xe, 1),1)];

b=inv(Xe'*Xe)*Xe'*y;
theta=-atan(b(1));
ct = cos(theta);
st = sin(theta);

%transform=struct('c', 1, 'R', [ct -st; st ct], 't', t);
transform=struct('c', 1, 'R', eye(2), 't', t)
