function plotSlatMutualInformation(prob, model, soln)
%plotSlatMutualInformation(prob, model, soln)
%plots the mutual information among the cameras at the last step

nd=prob.nCams*(prob.nCams-1)/2;
d=zeros(1,nd);
info=zeros(1,nd);

k=1;

cov=soln.Pt{end};

for i=1:prob.nCams,
  i
  for j=i+1:prob.nCams,
    d(k)=norm(prob.calib(i).pos-prob.calib(j).pos);
    info(k)=gMutualInformation(cov, model.ci(i,:),model.ci(j,:));
    k=k+1;
  end
end

plot(d,info,'x');

xlabel('d(i,j) [m]');
ylabel('I[\it{C}_{\it{i}}; \it{C}_{\it{j}} | \bf{o}_{1:\it{t}}]');
handles=[gca get(gca, 'xlabel') get(gca, 'ylabel') get(gca, 'zlabel')];
for h=handles,
  set(h, 'fontname', 'times');
  set(h, 'fontsize', 24);
end

print('-deps','slat-correlations.eps');
