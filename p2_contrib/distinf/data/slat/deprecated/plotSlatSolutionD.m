function plotSlatSolutionD(prob, xd, xv, yv, pv, mut, sigmat)
[a,b,c,nt]=size(xd);

nFigures=plotSlatProblem(prob);

for t=1:nt,
  figure(nFigures+1);
  mass=sum(sum(sum(xd(:,:,:,t))));
  surf(xv,yv,(sum(xd(:,:,:,t),3)')/mass);xlabel('x');ylabel('y');
  if exist('mut','var')
    mu=mut(:,t);
    sigma=sigmat(:,:,t);
  else
    [mu,sigma]=discrete2gaussian(xd(:,:,:,t), xv, yv, pv);
  end
  figure(nFigures+2);
  plotGaussian2D(mu(1:2),sigma(1:2,1:2));
  xlabel('x');ylabel('y');
  title(['visible for ' num2str(t) ' time step(s)']);
  figure(nFigures+3);
  plotGaussian2D(mu,sigma,[1 3]);
  xlabel('x');ylabel('pan');
  title(['visible for ' num2str(t) ' time step(s)']);
  ginput(1);
end

