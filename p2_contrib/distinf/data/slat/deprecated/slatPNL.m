function [xt, Pt, ecalib] = slatPNL(problem, model)
% [xt, Pt, ecalib] = slatKF(problem, model)
% see also function generateSlatProblem and generateSlatModel

if problem.nCams ~= model.nCams,
  error('The number of cameras in the problem and the model is different!!');
end

dbn = model.dbn;
evidences = cell(1, problem.nSteps);
%md = GetModelDomain(dbn);

for t=1:problem.nSteps,
  %t
  visiblei = find(problem.visible(:,t));
  obsNodes = model.sni(visiblei);
  obsValues = problem.obs(:,t,visiblei);
  obsSizes  = cell2mat({problem.calib(visiblei).imageSize});
  %osize(t)=length(obsValues(:));
  %nsize(t)=length(obsNodes);
  evidences{t}=CEvidenceCreate(dbn, obsNodes, obsValues(:)./obsSizes(:));
  %CEvidenceCreateByModelDomain(md, sliceObsNodes
end

%    factor = model.pcpd; %GetFactor(dbn, model.pni+model.nps);
%    AttachMatrix(factor, [1 0 1 0; 0 1 0 1; 0 0 1 0; 0 0 0 1], 'matWeights' ,0);

infEngine = C1_5SliceJtreeInfEngine(dbn)
DefineProcedure(infEngine, 'ptFiltering');

for t=1:problem.nSteps,
  t
  query = [model.pni model.cni];% + model.nps*(t>1);
  if t>1 % update the transition distribution according to time
    dt = problem.time(t)-problem.time(t-1);
    A  = model.A+model.Adt*dt;
    Q  = model.Q+model.Qdt*dt*dt;
    factor = GetFactor(dbn, model.pni+model.nps);
    AttachMatrix(factor, A(model.pi, model.pi), 'matWeights',0);
    AttachMatrix(factor, Q(model.pi, model.pi), 'matCovariance');
  end
  
  % todo: linearize the observation distributions
  
  EnterEvidence(infEngine, {evidences{t}});
  Filtering(infEngine, t-1);
  MarginalNodes(infEngine, query);
  potential = GetQueryJPD(infEngine);
  disp(GetDistributionType(potential));
  Dump(potential);
  xt(:,t) = GetMatrix(potential, 'matMean');
  Pt(:,:,t) = GetMatrix(potential, 'matCovariance');
end


disp('done');


muX=model.x0;
%muX(1:2)=problem.pos
P=model.P0;
n=length(muX);

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
  
  for k=1:problem.nCams,
    if(problem.visible(k,t))
      % Compute the approximate joint p(parents(Y),Y)
      R = diag([problem.sigma_u problem.sigma_v] ./ ...
          problem.calib(k).imageSize).^2;
      pi = [1:2 model.ci(k,:)];
      [mu, sigma, muY, sigmaY] = approximateJoint(muX(pi), P(pi,pi), ...
          @fnProjection,  R, model.param(k));
      
      % Now extend this joint to p(X,Y)
      [a, A] = gJoint2Conditional(mu, sigma, 1:length(pi));
      Afull = zeros(2,n); Afull(:,pi)=A;
      covXY = P*Afull';
      mu    = [muX; muY];
      sigma = [P covXY; covXY' sigmaY];
      
      % Condition on the observation
      [b, B, S] = gJoint2Conditional(mu, sigma, n+(1:2));
      muX = b+B*(problem.obs(:,t,k)./(problem.calib(k).imageSize'));
      P   = S;
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

%plotSlatSolution(problem, model, xt, Pt, ecalib);
