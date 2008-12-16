s=loadsoln('bp/emulab-9','20x20');
s1=loadsoln('bprand/emulab-9','20x20');
figure(1);
plotres('nupdates',s,s1)
legend('bp','bprand');
print -dpng plots/20x20-nupdates
xlim([0 1.5e4]);
figure(2);
plotres('totalcomm',s,s1)
xlabel('total communication [kB]');
legend('bp','bprand');
xlim([0 200]);
print -dpng plots/20x20-totalcomm

