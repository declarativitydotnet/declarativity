% s=loadsoln('bp/emulab-9','10x10-5/0')
% s1=loadsoln('bprand/emulab-9','10x10-5/0')
% s2=loadsoln('bprand-2/emulab-9','10x10-5/0')
% s5=loadsoln('bprand-5/emulab-9','10x10-5/0')

figure(1);
plotres('nupdates',s,s1,s2,s5);
legend('bp','bprand','bprand-2','bprand-5');
print('-dpng','plots/10x10-5-01-nupdates');

figure(2);
plotres('totalcomm',s,s1,s2,s5);
legend('bp','bprand','bprand-2','bprand-5');
print('-dpng','plots/10x10-5-01-totalcomm');

figure(3);
plotconv('totalcomm',0.05,s,s1,s2,s5);
legend('bp','bprand','bprand-2','bprand-5');
print('-dpng','plots/10x10-5-01-totalcomm-conv');

plotsingle(s1(1),3)
figure(4);
print('-dpng','plots/10x10-5-01-bprand-bandwidth');
