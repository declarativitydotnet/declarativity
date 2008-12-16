function plotSlatMF1(prob, xmt, offset, mut, sigmat)
nt=length(xmt);

if ~exist('offset','var')
  offset=0;
end

%offset = length(xmt(1).mu(:,1))-2

nFigures=plotSlatProblem(prob);

for t=1:nt,
  figure(nFigures+1);
  plotMixtureGaussian2D(xmt(t).p, xmt(t).mu, xmt(t).sigma, offset+(1:2),xmt(t).pan);
  xlabel('x');ylabel('y');
  if exist('mut','var')
    mu=mut(:,t);
    sigma=sigmat(:,:,t);
  else
    [mu,sigma]=mixture2gaussian(xmt(t).p, xmt(t).mu, xmt(t).sigma, xmt(t).pan);
  end
  print('-dpng',['mixture-' num2str(t) '.png']);

  figure(nFigures+2);
  plotGaussian2D(mu,sigma,offset+(1:2));
  xlabel('x');ylabel('y');
  title(['visible for ' num2str(t) ' time step(s)']);
  %print('-dpng',['xy-' num2str(t) '.png']);

  figure(nFigures+3);
  plotGaussian2D(mu,sigma,offset+[1 3]);
  xlabel('x');ylabel('pan');
  title(['visible for ' num2str(t) ' time step(s)']);
  %print('-dpng',['xpan-' num2str(t) '.png']);
  ginput(1);
end

