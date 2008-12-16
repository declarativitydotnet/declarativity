algorithms = {'synchronous/5' 'synchronous/2' 'synchronous/1' 'synchronous/0.3' 'synchronous/0' ... %'asynchronous' ...
              'residual'};
clear s1 s5
for i=1:length(algorithms)
  fprintf('%s\n',algorithms{i});
  %s1{i} = loadsoln(algorithms{i}, '10x10-1/');
  s5{i} = loadsoln(algorithms{i}, '10x10-5/');
end

% figure(1);
% plotres('time', s1{:});
% legend(algorithms{:});
% title('10x10_1')
% print('-dpng','plots/10x10_1_centralized_time');
% 
% figure(2);
% plotres('nupdates', s1{:});
% legend(algorithms{:});
% title('10x10_1')
% print('-dpng','plots/10x10_1_centralized_nupdates');

figure(3);
plotres('time', s5{:});
legend(algorithms{:});
title('10x10_5')
print('-dpng','plots/10x10_5_centralized_time');

figure(4);
plotres('nupdates', s5{:});
legend(algorithms{:});
title('10x10_5')
print('-dpng','plots/10x10_5_centralized_nupdates');
