function plotSlatObservations(problem, soln)
%plotSlatObservations(problem, soln)

calib=problem.calib;

a=sqrt(problem.nCams/(4/3));
b=ceil(a*4/3);
a=ceil(problem.nCams/b);
clf;
steps = [1 20:20:problem.nSteps];
for k=1:problem.nCams,
  subplot(a,b,k);
  axis equal;
  axis([0 calib(k).imageSize(1)-1 0 calib(k).imageSize(2)-1]);
  set(gca,'YDir', 'reverse');
  hold on;
  grid on;
  if isfield(problem,'projection')
    plot(problem.projection(1,:,k), problem.projection(2,:,k),'ro-');
  end
  obs2=problem.obs(:,:,k);
  obs2(:,find(problem.visible(k,:)==0))=nan;
  plot(obs2(1,:),obs2(2,:), 'mo-');

  %legend('Exact projection', 'Observation', 'Exact w/ ecalib');
  for t=steps,
    if problem.visible(k,t)
      h=text(problem.obs(1,t,k)+5,problem.obs(2,t,k)-5,num2str(t));
      set(h,'backgroundcolor',[1 1 1]);
    end
  end
  title(['Camera ' num2str(problem.cameraid(k))]);        
  hold off;
end
