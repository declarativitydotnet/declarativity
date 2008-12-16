function absbel = to_absolute(model, bel, oldmodel)
% Converts the belief (a MG) to absolute representation

if ~exist('oldmodel','var')
  oldmodel=legacy(model);
end

if isa(bel,'decomposable_model')
  marginals = [bel.marginals];
  for i=1:length(marginals)
    newmarg(i) = to_absolute(model,marginals(i),oldmodel);
  end
  absbel = decomposable_model(newmarg);
else
  vars = posevars(model);
  bvars1 = intersect(args(bel),vars(1,:));
  cams = find(vars(1,:),bvars1);
  bel = moment_gaussian(bel);
  lvar=  locvars(model); lvar=lvar(1);
  ctr = centervars(model);

  for i=1:length(cams)
    k = cams(i);
    myvars = [lvar ;vars(:,k)]';
    muX = mean(bel,myvars);
    sigmaX = covariance(bel,myvars);
    [mu,sigma,muY,sigmaY] = approximateJoint(muX,sigmaX,@fnUvCameraCenter, ...
      eye(2)*1e-6, oldmodel.param(k));
    %   [mu,sigma] = approximateJoint(muX,sigmaX,@fnUvCameraCenter, ...
    %     eye(2)*1e-6, oldmodel.param(k));
    [a0,A,S] = gJoint2Conditional(mu,sigma,1:length(muX));
    bel = combine(bel, moment_gaussian(ctr(k), a0, S, myvars, A));
  end

  newvars = absposevars(model,cams);
  newvars = [locvars(model);newvars(:)];

  absbel = marginal(bel,newvars);
end