async=loadsoln('asynchronous','10x10_1/');
rbp=loadsoln('residual','10x10_1/');

figure(1);
plotres('time', async, rbp);
legend('asynchronous','rbp');
title('10x10, C=1');
print('-dpng', '10x10_1_residuals.png');

figure(2);
plotconv('time', 1e-3, async, rbp);
legend('asynchronous','rbp', 'location', 'northwest');
title('10x10, C=1');
print('-dpng', '10x10_1_convergence.png');

async=loadsoln('asynchronous','10x10_5/');
rbp=loadsoln('residual','10x10_5/');
figure(3);
plotres('time', async, rbp);
legend('asynchronous','rbp');
title('10x10, C=5');
print('-dpng', '10x10_5_residuals.png');

figure(4);
plotconv('time', 1e-3, async, rbp);
legend('asynchronous','rbp', 'location', 'northwest');
title('10x10, C=5');
print('-dpng', '10x10_5_convergence.png');
