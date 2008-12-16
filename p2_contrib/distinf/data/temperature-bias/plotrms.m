function plotrms(prob, s)
plot(s.time, s.rms);
[cg,t,b]=joint(prob, s.n);
crms = sqrt(sum((mean(cg,b)-prob.bias(1:s.n)).^2));
hold on
plot(s.time, ones(size(s.time))*crms,'--');
hold off


