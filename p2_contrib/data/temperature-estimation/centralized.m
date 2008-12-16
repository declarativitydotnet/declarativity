function s=centralized(probname, n)
import prl.*;

load(probname);
pfactors = load_model(probname, n);
ss=shafer_shenoy(pfactors);
ss.calibrate;

tv = tempvars(n);

for i=1:n,
  s.temp(i) = ss.belief(domain(tv(i))).as_moment_gaussian.mean;
end

s.rms=rms(model.tmean(1:n)', s.temp);
