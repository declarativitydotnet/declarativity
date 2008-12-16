algorithms = {'synchronous/1' 'synchronous/0.3' 'synchronous/0' 'asynchronous' ...
              'residual' 'exponential/1' 'exponential/2' 'exponential/10'};
clear s
for i=1:length(algorithms)
  s{i} = loadsoln(algorithms{i}, '20x20');
end

figure(1);
plotres('time', s{:});
legend(algorithms{:});
print('-dpng','plots/20x20_centralized_time');

figure(2);
plotres('nupdates', s{:});
legend(algorithms{:});
print('-dpng','plots/20x20_centralized_nupdates');
