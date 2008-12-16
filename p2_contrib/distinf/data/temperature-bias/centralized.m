function s=centralized(probname, n)
import prl.*;
load(probname);

[pfactors, likelihoods] = load_model(probname, n);
ss=shafer_shenoy([pfactors likelihoods]);
ss.calibrate;

bv = biasvars(n);
tv = tempvars(n);

for i=1:n,
  s.temp(i) = ss.belief(domain(tv(i))).as_moment_gaussian.mean;
  s.bias(i) = ss.belief(domain(bv(i))).as_moment_gaussian.mean;
end

s.rms = rms(model.bias(1:n)', s.bias);
