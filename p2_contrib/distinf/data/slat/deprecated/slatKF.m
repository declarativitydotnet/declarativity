function soln = slatKF(problem, model, push_in)
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
Pt=cell(1,problem.nSteps);

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
  for k=visibles,
    if(problem.visible(k,t))
      % Get prior for parents(Y) and adjust if necessary
      if isfield(model, 'R')
        R = model.R;
      else
        R = diag([problem.sigma_u problem.sigma_v]);
      end
      %R
      pi = [1:2 model.ci(k,:)];

       if ~seen(k),
         disp(['Camera ' num2str(k) ' made its first observation.'])
         if ~foundVisible
             foundVisible = 1;
             muXv=muX(pi);
             Pv=P(pi,pi);
             muXv(3:4)=problem.calib(k).pos(1:2); % this is a hack
             Pv(3:4,3:4)=eye(2)*1e-4; %1e-1; %both 
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
             param.P      = [0;0;1.8];
             [mu,sigma] = approximateGaussian(problem.obs(:,t,k), ...
                R*4, @fnCameraCenter, zeros(2), param);
             % N(mu, sigma) is over Ix,Iy,u,v
             %muXv(1:2) remains the same
             muXv(3:4)=muXv(1:2);
             Pv(3:4,1:2)=Pv(1:2,1:2);
             Pv(1:2,3:4)=Pv(1:2,1:2)';
             Pv(3:4,3:4)=eye(2)*norm(mu-muXv(1:2),2)^2+Pv(1:2,1:2);
             %muXv(5:6)=mu;
             %muXv(7); remains unchanged
             %Pv(1:2)  remains unchanged;
             %Pv(3:4,3:4)=Pv(1:2,1:2)+eye(2)*1e-4; % C=L+epsilon
             %Pv(5:6,5:6)=sigma;
             %Pv(7,7) unchanged
         end
         %save test
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

          % Compute the approximate posterior  p(parents(Y)|Y=y)
          disp(['Linearizing camera ' num2str(k) ' observation...'])
          if push_in>0
            [mu, sigma] = approximatePosterior(muXv, Pv, ...
              @fnProjection, R, problem.obs(:,t,k), model.param(k), push_in);
          else
            [mu, sigma] = approximatePosteriorUV(muXv, Pv, ...
               @fnProjection, R, problem.obs(:,t,k), model.param(k));
          end
          

          % Now extend this joint to p(X)
          [a, A, S] = gJoint2Conditional(muX, P, pi);
          muX1 = A*mu + a;
          P1   = A*sigma*A' + S;
          covXv= A*sigma;
          pic = setdiff(1:length(muX),pi);
          
          muX=zeros(length(muX),1);
          P=zeros(length(muX));
          muX(pi)=mu;
          muX(pic)=muX1;
          P(pi,pi)=sigma;
          P(pic,pic)=P1;
          P(pic,pi)=covXv;
          P(pi,pic)=covXv';
          if k==1,
            mu(1:2)
          end
      end    
    end
  end
  
  % Store the result
  xt(:,t)=muX;
  Pt{t}=P;
  
  if mod(t,50)==0
    save test
  end
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
