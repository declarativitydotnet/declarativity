load model
bar(model.tmean(setdiff(1:54,5)),'b')
set(gca,'fontsize',24);
ylabel('temperature [C]');
xlabel('location');
xlim([0 54]);
print -dpng plots/means.png
