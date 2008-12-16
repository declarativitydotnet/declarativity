function plotrms(algo, version)
probname = version;
load(probname)
csoln = loadsoln('residual',probname);
dsoln=loadsoln(algo, probname);

figure(1);
plotimage(img, csoln);

figure(2);
plotimage(img, dsoln);

figure(3);
plot(dsoln.nupdates, rms(dsoln.beliefs, img)');
hold on;
plot(csoln.nupdates, rms(csoln.beliefs, img), 'r--');
hold off;
%xlim([0 20])
xlabel('number of updates [s]')
ylabel('RMS error');
legend('distributed', 'centralized');

