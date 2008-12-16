function plot_nips(usecache, type)

if usecache
  load plot_nips
else
  sc=loadsoln('residual','10x10-5/');
  s0=loadsoln('bp/emulab-9','10x10-5/');
  s1=loadsoln('bprand-1/emulab-9','10x10-5/');
  s3=loadsoln('bprand-3/emulab-9','10x10-5/');
  s5=loadsoln('bprand-5/emulab-9','10x10-5/');
  save plot_nips sc s0 s1 s3 s5
end

figure(1);
plotres('nupdates',s1,s3,s5,sc);
xlim([0 2e4]);
ylim([0 1]);
set(gca, 'fontsize', 24, 'fontname', 'times');
xlabel('number of updates');
ylabel('residual');
legend('\rho = 1', '\rho = 3', '\rho = 5', 'centralized');
if nargin>1
  print(['-d' type], 'plots/rbp-exponent');
end

figure(2);
plotres('totalcomm',s0,s5);
xlim([0 2000]);
ylim([0 1]);
set(gca, 'fontsize', 24, 'fontname', 'times');
xlabel('total communication [kB]');
ylabel('residual');
legend('synchronous','randomized');
if nargin>1
  print(['-d' type], 'plots/random-ising-comparison');
end

figure(3);
% Compute the bandwidth statistics
[hb,ht]=plotbwbars('residual', 0.1, [3 6 5], s5, 'randomized', s0, 'synchronous');
set([gca ht], 'fontsize', 24, 'fontname', 'times');
legend('spanning tree','aggregation','inference','total','location','northwest');
ylabel('total communication [kB]');
if nargin>1
  print(['-d' type], 'plots/random-ising-bandwidth');
end
