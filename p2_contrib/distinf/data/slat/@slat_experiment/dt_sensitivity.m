function experiments = dt_sensitivity(ex, dt, repeated)
% function dt_sensitivity(ex, dt, repeated)
% takes in a single experiment and returns a |dt| x |repeated| array of
% experiments, in which we vary dt according to dt for n times

s = get(ex);
oe  = s.evidence.algorithm;
dat = s.evidence.data;
ev = s.evidence;

if isa(s.evidence.algorithm, 'hsv_detector')
  
  % hack this for now

  if ~exist('repeated')
    repeated = ev;
  end
  
  for i=1:length(dt)
    for j=1:length(repeated)
      ev = repeated(j);
      oe  = ev.algorithm;
      obs = ev.obs;
      obs(find(ev.obs<0))=nan;  
  
      oe = set(oe, 'dt', dt(i));
      dt_time = ev.time(1):dt(i):ev.time(end); 
      dt_obs  = interp1(ev.time, shiftdim(obs,1), dt_time);
      dt_visible = ~isnan(dt_obs(:,:,1))';
      dt_obs(find(isnan(dt_obs))) = -1;
      dt_ev = evidence(dat, oe, dt_visible, shiftdim(dt_obs,2), dt_time);
      experiments(i,j)=experiment(dt_ev, s.algorithm, s.model, s.scenario);
    end
  end

else
  
  for i=1:length(dt),
    oe = set(oe, 'dt', dt(i));
    ev = run(oe, dat);
    experiments(i) = experiment(ev, s.algorithm, s.model, s.scenario);
  end
  
  experiments=experiments';

  if exist('repeated', 'var')
    experiments = repeat(experiments, repeated);
  end
end

