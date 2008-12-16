function [xt, Pt, ecalib, xm, xmt] = slatMF3(problem, model)
% [xt, Pt, ecalib] = slatKF(problem, model)
% see also function generateSlatProblem and generateSlatModel

% This function uses mixture of Gaussians to do the filtering.
% The cliques for the burning-in cameras contain all of the variables.

if problem.nCams ~= model.nCams,
  error('The number of cameras in the problem and the model is different!!');
end

%xt(:,1)=x0;
%Pt(:,:,1)=P0;

muX=model.x0;
%muX(1:2)=problem.pos
P=model.P0;
n=length(muX);
nPans = 45;

seen = zeros(1,model.nCams);
foundVisible=0;

xt=zeros(n,problem.nSteps);
Pt=zeros(n,n,problem.nSteps);

for t=1:problem.nSteps,
  t
  
  % Prediction (unless it's the first step)
  if t~=1
    dt = problem.time(t)-problem.time(t-1);
    A  = model.A+model.Adt*dt;
    Q  = model.Q+model.Qdt*dt*dt;
    muX=A*muX;
    P=A*P*A'+Q;
  end
  
  for k=find(problem.visible(:,t))',
    R = diag([problem.sigma_u problem.sigma_v] ./ ...
        problem.calib(k).imageSize).^2;
    pi1 = [1:2 model.ci(k,:)];
    ci = model.ci(k,:);

    if ~seen(k), % Initialize the mixture model

      % Compute the initial distribution (based on the first
      % observation)
      if ~foundVisible
        x.pan = problem.calib(k).pan;
        x.pan2= problem.calib(k).pan;
      else
        x.pan = -pi:2*pi/nPans:pi-1e-6;
        x.pan2= x.pan;
        x.pan2(ceil(nPans/2)+1:end)=x.pan(ceil(nPans/2)+1:end)-2*pi;
      end
      
      for i=1:length(x.pan),
        param(i).KK     = problem.calib(k).KK;
        param(i).height = problem.calib(k).pos(3);
        param(i).R      = panTilt2R(x.pan(i), problem.calib(k).tilt);
        param(i).P      = [muX(1:2);1.8];
        %[mu, sigma] = approximateJoint([problem.obs(:,t,k);muX(1:2)], ...
        %    [eye(2)*9 zeros(2);zeros(2) P(1:2,1:2)], ...
        %    @fnCameraCenter2, zeros(2), param(i));
        %x.mu(:,i) = mu(3:end);
        %x.sigma(:,:,i) = sigma(3:end, 3:end);
        [mu,sigma] = approximateGaussian(problem.obs(:,t,k), ...
            eye(2)*100, @fnCameraCenter, P(1:2,1:2), param(i));
        x.mu(:,i)=muX;
        x.sigma(:,:,i)=P;
        x.mu(pi1(3:4),i)=mu;
        x.mu(pi1(5),i)=x.pan(i);
        x.sigma(pi1(3:4),pi1(3:4),i)=sigma;
        x.sigma(pi1(5),pi1(5),i)=1e-6;
        %x.mu(:,i)=[muX(1:2);mu];
        %x.sigma(:,:,i)=[P(1:2,1:2) zeros(2);zeros(2) sigma];%mu=[x.p;param
      end
      x.p = ones(1,length(x.pan))/length(x.pan);
      %[x.mu,x.sigma] = approximateGaussian(problem.obs(:,t,k), ...
      %    eye(2)*9, @fnCameraCenter, P(1:2,1:2), param);
      %[mut(:,1) sigmat(:,:,1)]=mixture2gaussian(x.p,x.mu,x.sigma,x.pan);

      % Compute the camera parameters for the observation likelihood
      for i=1:length(x.pan),
        x.param(i).repr = 1;
        x.param(i).cposi = [3 4 0];
        x.param(i).pani  = 0;
        x.param(i).tilti = 0;
        x.param(i).calib = problem.calib(k);
        x.param(i).calib.pan = x.pan(i);
        x.param(i).trim = 1;
      end

      seen(k)=1;
      xm(k)=x;
      xmt{k}(1)=xm(k);
    
    else
      obs = problem.obs(:,t,k)./(problem.calib(k).imageSize');
        
      % Do an update
      if seen(k)<100 % Mixture update
        x=xm(k);

        for i=1:length(x.pan),
          if x.p(i)>0
            A  = model.A+model.Adt*dt;
            Q  = model.Q+model.Qdt*dt*dt;
            %A  = A(pi1(1:4),pi1(1:4)); Q=Q(pi1(1:4),pi1(1:4));
            x.mu(:,i)=A*x.mu(:,i);
            x.sigma(:,:,i)=A*x.sigma(:,:,i)*A'+Q;
    
            c1=model.ci(k,:);
            c2=setdiff(1:n,c1);
            [K,h]=moment2canon(x.mu(:,i), x.sigma(:,:,i));
            [Kv,hv]=moment2canon(muX, P);
            Kv(c1,c1)=0;
            hv(c1)=0;
            [Ko1,ho1]=moment2canon(x.mu(c2,i), x.sigma(c2,c2,i));
            Ko=zeros(n); Ko(c2,c2)=Ko1;
            ho=zeros(n,1); ho(c2)=ho1;
            
            K=K-Ko+Kv;
            h=h-ho+hv;
            if min(eig(K)<=0),
              warning(['K is not P.D. (lambda_min=' num2str(min(eig(K))) ').'])
            end
            [mu, sigma] = canon2moment(K,h);
            mu1=mu;sigma1=sigma;
            
            % Compute the approximate joint p(parents(Y),Y)
            [mu, sigma, muY, sigmaY] = approximateJoint(mu(pi1), sigma(pi1,pi1), ...
                @fnProjection, R, model.param(k));
            
            % Now extend this joint to p(X,Y)
            [a, A] = gJoint2Conditional(mu, sigma, 1:length(pi1));
            Afull = zeros(2,n); Afull(:,pi1)=A;
            covXY = sigma1*Afull';
            mu    = [mu1; muY];
            sigma = [sigma1 covXY; covXY' sigmaY];
        
            % Condition on the observation
            [b, B, S] = gJoint2Conditional(mu, sigma, n+(1:2));
            x.mu(:,i) = b+B*obs;
            x.sigma(:,:,i) = S;

            x.p(i)=x.p(i)*mvnpdf(obs', muY', sigmaY);
          end
        end
        x.p=x.p/sum(x.p);
        
        xm(k)=x;
        xmt{k}(end+1)=x;
        seen(k)=seen(k)+1;
        
        % pass message over to the main clique
        %[muXv, Pv] = mixture2gaussian(xm(k).p, xm(k).mu, ...
        %    xm(k).sigma, xm(k).pan);
        %muXv=muXv(1:2);
        %Pv=Pv(1:2,1:2);
        %    
        %[K,h]=moment2canon(muX, P);
        %[Ko1,ho1]=moment2canon(muX(1:2), P(1:2,1:2));
        %Ko=zeros(n); Ko(1:2,1:2)=Ko1;
        %ho=zeros(n,1); ho(1:2)=ho1;
        %[Kv1,hv1]=moment2canon(muXv, Pv);
        %Kv=zeros(n); Kv(1:2,1:2)=Kv1;
        %hv=zeros(n,1); hv(1:2)=hv1;
        %    
        %K=K-Ko+Kv;
        %h=h-ho+hv;
        %if min(eig(K)<=0),
        %  warning(['K is not P.D. (lambda_min=' num2str(min(eig(K))) ').'])
        %end
        % 
        % [muX, P] = canon2moment(K,h);
            
      else % Regular update

        muXv = muX(pi1);
        Pv = P(pi1, pi1);
        
        % Compute the approximate joint p(parents(Y),Y)
        [mu, sigma, muY, sigmaY] = approximateJoint(muXv, Pv, ...
            @fnProjection, R, model.param(k));
        
        % Now extend this joint to p(X,Y)
        [a, A] = gJoint2Conditional(mu, sigma, 1:length(pi1));
        Afull = zeros(2,n); Afull(:,pi1)=A;
        covXY = P*Afull';
        mu    = [muX; muY];
        sigma = [P covXY; covXY' sigmaY];
        
        % Condition on the observation
        [b, B, S] = gJoint2Conditional(mu, sigma, n+(1:2));
        muX = b+B*obs;
        P   = S;
      end
      
    end

    if foundVisible & (seen(k)>0 & seen(k)<100)
      a=cumsum(sort(xm(k).p, 2, 'descend'));
      if a(6)>0.95
        foundVisible = 0
      end
    end
    
    % Pass the mixture distribution over to the main parameters.
    if ~foundVisible 
      
      foundVisible = 1;
      seen(k)=100;

      c1=model.ci(k,:);
      [stuff,i]=max(xm(k).p);
      if abs(xm(k).pan(i))>pi/2 % handle wrap-arounds
        xm(k).mu(c1(3),:)=xm(k).pan2;
        %[muXv, Pv] = mixture2gaussian(xm(k).p, xm(k).mu, ...
        %    xm(k).sigma); %xm(k).pan);
      end
      [muXv, Pv] = mixture2gaussian(xm(k).p, xm(k).mu, ...
          xm(k).sigma);

      disp(['Camera ' num2str(k) ': pos-epos=' ...
            mat2str(problem.calib(k).pos(1:2)-muXv(c1(1:2)))])

      %if Pv(5,5)==0
      %  Pv(5,5)=1e-6;
      %end

      %c1=model.ci(k,:);
      %c2=setdiff(1:n,c1);

      %[K,h]=moment2canon(muX, P);
      %[Ko1,ho1]=moment2canon(muX(pi1), P(pi1,pi1));
      %Ko=zeros(n); Ko(pi1,pi1)=Ko1;
      %ho=zeros(n,1); ho(pi1)=ho1;
      %[Kv1,hv1]=moment2canon(muXv, Pv);
      %Kv=zeros(n); Kv(pi1,pi1)=Kv1;
      %hv=zeros(n,1); hv(pi1)=hv1;
        
      %K=K-Ko+Kv;
      %h=h-ho+hv;
      %if min(eig(K)<=0),
      %  warning(['K is not P.D. (lambda_min=' num2str(min(eig(K))) ').'])
      %end
        
      %[muX, P] = canon2moment(K,h);
      muX=muXv;
      P=Pv;
    end
  end
  
  % Store the result
  xt(:,t)=muX;
  Pt(:,:,t)=P;
  last=t;
end

% Compute the estimated calibration matrices
for k=1:problem.nCams,
  ecalib(k)=problem.calib(k);
  [stuff ecalib(k).R ecalib(k).T] = ...
    decodeCameraState(xt([1:2 model.ci(k,:)],last), model.param(k));
end

%plotSlatSolution(problem, model, xt, Pt, ecalib);
