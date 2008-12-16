algorithm = ...
  {'asynchronous' 'residual' 'bp/emulab-9' 'bpdht/emulab-9'};

clear s1 s5           
for i=1:length(algorithm)
  algorithm{i}       
  %s1{i} = loadsoln(algorithm{i}, '10x10-1/01');
  s5{i} = loadsoln(algorithm{i}, '10x10-5/01');
end


figure(1);
plotres('nupdates', s5{:});
legend(algorithm{:});
title('10x10_5/01')
print('-depsc','plots/10x10_5_01_nupdates_residual');

% figure(2);
% plotconv('nupdates', 1e-3, s5{:});
% legend(algorithm{:});
% title('10x10_5')
% print('-depsc','plots/10x10_5_nupdates_converged');

%figure(3);
%plotres('bandwidth', s5{3:end})
%legend(algorithm{3:end});
%title('10x10_5')
%print('-depsc','plots/10x10_5_bandwidth_residual');

%figure(4);
%plotres('nupdates', s1{:});
%legend(algorithm{:});
%title('10x10_1')
%print('-depsc','plots/10x10_1_nupdates_residual');

%figure(5);
%plotconv('nupdates', 1e-3, s1{:});
%legend(algorithm{:});
%title('10x10_1')
%print('-depsc','plots/10x10_1_nupdates_converged');
%figure(6);
%plotres('bandwidth', s1{3:end})
%legend(algorithm{3:end});
%title('10x10_1')
%print('-depsc','plots/10x10_1_bandwidth_residual');