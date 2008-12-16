function prob = generate_observations(prob, obs_var)
% prob = generate_observations(prob, obs_var)
% Adds the observations for a SLAT problem
  
n=prob.nsteps;

for k=1:prob.ncams,
  calib = prob.calib(k);
  p = calib.KK * [calib.R calib.T] * [prob.pos;ones(1,n)];
  visible = p(3,:) > 0;
  p = p(1:2,:)./(ones(2,1)*p(3,:));
  o = p + randn(2,n) * sqrt(obs_var);
  visible = o(1,:)>=0 & o(1,:)<calib.imageSize(1) & ...
            o(2,:)>=0 & o(2,:)<calib.imageSize(2) & ...
            visible;
  p(:, ~visible) = nan;
  o(:, ~visible) = nan;
  prob.proj{k} = p;
  prob.obs{k}  = o;
  prob.visible(k,:) = visible;
end
prob.obs_var = obs_var;


