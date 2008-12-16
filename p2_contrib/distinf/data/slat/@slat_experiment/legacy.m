function [prob,model]=legacy(object)
%function [prob,model]=legacy(object)
% Returns a legacy representation of the problem

s=get(object);
tb=testbed(object);
dat=s.evidence.data;
ev=s.evidence;

prob.calib=s.model.calib;
prob.nCams=ncams(tb);
prob.range=tb.range
prob.sigma_u=sqrt(s.model.obs_var);
prob.sigma_v=sqrt(s.model.obs_var);
prob.pos=dat.trajectory;
prob.nSteps=nsteps(s.evidence);
prob.time=s.evidence.time;
%prob.projection=
%prob.noise
prob.obs=s.evidence.obs;
prob.visible=s.evidence.visible;
%prob.adjacency=



model=legacy(s.model);
