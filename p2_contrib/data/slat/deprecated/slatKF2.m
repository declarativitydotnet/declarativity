function soln = slatKF2(problem, model)
% [xt, Pt, ecalib] = slatKF(problem, model)
% see also function generateSlatProblem and generateSlatModel

if problem.nCams ~= model.nCams,
  error('The number of cameras in the problem and the model is different!!');
end

%xt(:,1)=x0;
%Pt(:,:,1)=P0;

muX=model.x0;
%muX(1:2)=problem.pos
P=model.P0;
n=length(muX);

seen = zeros(1,model.nCams);
foundVisible=0;

xt=zeros(n,problem.nSteps);
Pt=zeros(n,n,problem.nSteps);

for t=1:problem.nSteps
  t
  
  % Prediction (unless it's the first step)
  if t~=1
    dt = problem.time(t)-problem.time(t-1);
    A  = model.A+model.Adt*dt;
    Q  = model.Q+model.Qdt*dt*dt;
    muX=A*muX;
    P=A*P*A'+Q;
  end
  
  visibles=find(problem.visible(:,t))';
  visibles=visibles(end:-1:1);
  for k=visibles,
    if(problem.visible(k,t))
      % Get prior for parents(Y) and adjust if necessary
      R = diag([problem.sigma_u problem.sigma_v]);
      pi = [1:2 model.ci(k,:)];

      if ~seen(k),
        if ~foundVisible
            disp(['Camera ' num2str(k) ' is the first to observe the marker.'])
            foundVisible = 1;
            muXv=muX(pi);
            Pv=P(pi,pi);
            muXv(3:4)=problem.calib(k).pos(1:2); % this is a hack
            Pv(3:4,3:4)=eye(2)*1e-3;
            %muXv(3:4)=muXv(1:2);
            %Pv(3:4,3:4)=eye(2)*(norm(problem.calib(k).pos(1:2)-muXv(1:2))^2);
            %[muXv, Pv,p,w] = visibilitySampling(muXv, Pv, model.param(k), ...
            %    problem.obs(:,t,k)./(problem.calib(k).imageSize'), R);
            
            %muXv=muX(pi);
            %Pv=P(pi,pi);
        else
            muXv=muX(pi);
            Pv=P(pi,pi);
            param.KK     = problem.calib(k).KK;
            param.height = problem.calib(k).pos(3);
            param.R      = panTilt2R(0, problem.calib(k).tilt);
            param.P      = [muX(1:2);1.8];
            [mu,sigma]   = approximateGaussian(problem.obs(:,t,k), ...
               eye(2)*100, @fnCameraCenter, P(1:2,1:2), param);
            % N(mu, sigma) is over LX,LY,CT,ST
            ii=[1 2 5 6];
            newi=[1 2 5 6 3 4];
            dC=mu-muX(1:2);
            mu=mu-dC;
            A=[0 0 dC(1) dC(2); 0 0 dC(2) -dC(1)];
            [muXv,Pv]=gConditional2Joint(muXv(ii), Pv(ii,ii), mu, A, sigma);
            %reindex
            muXv=muXv(newi);
            Pv=Pv(newi,newi);
        end
        save test
        [K,h]=moment2canon(muX, P);
        [Ko1,ho1]=moment2canon(muX(pi), P(pi,pi));
        [Kv1,hv1]=moment2canon(muXv, Pv);
        Ko=zeros(n); Ko(pi,pi)=Ko1;
        ho=zeros(n,1); ho(pi)=ho1;
        Kv=zeros(n); Kv(pi,pi)=Kv1;
        hv=zeros(n,1); hv(pi)=hv1;
        
        K=K-Ko+Kv;
        h=h-ho+hv;
        
        if min(eig(K)<=0),
            warning(['K is not P.D. (lambda_min=' num2str(min(eig(K))) '). What now?']);
        end
        
        [muX, P] = canon2moment(K,h);
        seen(k)=1;
      else
          muXv = muX(pi);
          Pv = P(pi, pi);

          % Compute the approximate joint p(parents(Y),Y)
          disp(['Linearizing camera ' num2str(k) ' observation...'])
          [mu, sigma, muY, sigmaY] = approximateJoint2(muXv, Pv, ...
              @fnProjection, R, model.param(k));

          % Now extend this joint to p(X,Y)
          [a, A, S] = gJoint2Conditional(mu, sigma, 1:length(pi));
          
          Afull = zeros(2,n); Afull(:,pi)=A;
          [mu,sigma]=gConditional2Joint(muX, P, a, Afull, S);

          % Condition on the observation
          [b, B, S] = gJoint2Conditional(mu, sigma, n+(1:2));
          muX = b+B*(problem.obs(:,t,k));
          P   = S;
      end    
    end
  end
  
  % Store the result
  xt(:,t)=muX;
  Pt(:,:,t)=P;
end

% Compute the estimated calibration matrices
for k=1:problem.nCams,
  ecalib(k)=problem.calib(k);
  [stuff ecalib(k).R ecalib(k).T] = ...
    decodeCameraState(xt([1:2 model.ci(k,:)],problem.nSteps), model.param(k));
end

soln.xt=xt;
soln.Pt=Pt;
soln.ecalib=ecalib;
soln.burn_in=0;

%plotSlatSolution(problem, model, xt, Pt, ecalib);
