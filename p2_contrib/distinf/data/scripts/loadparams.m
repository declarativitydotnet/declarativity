function s=loadparams(name, nrecords)

a = load([name '.txt']);

assert(all(size(a)==[1 3]));
s.n = a(1);
s.epoch = a(2);
s.niters = a(3);

if ~exist('nrecords','var')
  s.t = (1:s.niters)*s.epoch;
else
  s.t = (1:nrecords)*s.epoch*s.niters/nrecords;
end
s.e = s.t / s.epoch;
