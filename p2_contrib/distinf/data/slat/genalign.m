function ex=generateSlatAlign(id)
% problem=generateSlatHallway(KK,id,...)
% the optional parameters fps, mps are passed to generateSlatProblem
% ???
 
% From one of the fire-i camereas
KK = [862.4086         0  314.4831; ...
      0  860.4891  213.8647; ...
      0         0    1.0000];

if id==1                         % A line of cameras (hall11)
  pos=[0.75*(-1:1); ...
       zeros(1,3); ...
       ones(1,3)*3.5;
       ones(1,3)*pi/2;
       ones(1,3)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  problem=generateSlatProblem(calib,'range',[-2 2 -2 2],'fps',4);
  ev = importSimulatedProblem(problem, 'align1');
  algorithm = boyen_koller;
  model = slat_model(ev.data.testbed, 'obs_var',400, ...
                     'assumed_clusters',{[1 2],[2 3]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==2   
  % A slightly more involved experiment, in which both cliques
  % receive observations
  pos=[0.75*(-1:2); ...
       zeros(1,4); ...
       ones(1,4)*3.5;
       ones(1,4)*pi/2;
       ones(1,4)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  problem=generateSlatProblem(calib,'range',[-2 2 -2 2],'fps',4);
  ev = importSimulatedProblem(problem, 'align2');
  algorithm = boyen_koller;
  model = slat_model(ev.data.testbed, 'obs_var',400, ...
                     'assumed_clusters',{[1 2 3],[3 4]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==3
  % Experiment, in which two cameras receive observations of the
  % shared clique, but do not communicate
  pos=[0.25*(-1:1); ...
       [0 -.2 0]; ...
       ones(1,3)*3.5;
       ones(1,3)*pi/2;
       ones(1,3)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  problem=generateSlatProblem(calib,'range',[-2 2 -2 2],'fps',4);
  ev = importSimulatedProblem(problem, 'align3');
  algorithm = boyen_koller;
  model = slat_model(ev.data.testbed, 'obs_var',400, ...
                     'assumed_clusters',{[1 2 3],[1 2 3]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==4
  % An experiment similar to #2, in which the belief is aligned continuously
  pos=[0.75*(-1:2); ...
       zeros(1,4); ...
       ones(1,4)*3.5;
       ones(1,4)*pi/2;
       ones(1,4)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  problem=generateSlatProblem(calib,'range',[-2 2 -2 2],'fps',10);
  ev = importSimulatedProblem(problem, 'align4');
  algorithm = boyen_koller;
  model = slat_model(ev.data.testbed, 'obs_var',400, ...
                     'assumed_clusters',{[1 2],[3 4]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==5
  % An experiment similar to #4, in which the belief is aligned
  % continuously and the object takes a sharp turn
  
  pos=[0.75*(-1:2); ...
       zeros(1,4); ...
       ones(1,4)*3.5;
       ones(1,4)*pi/2;
       ones(1,4)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  tb = testbed('align5',{},[-2 2 -2 2],calib);
  trajectory = [(-10:15)/10;zeros(1,26);ones(1,26)*1.8];
  trajectory = [trajectory [1.5 1.5 1.5 1.5;0.1 0.2 0.3 0.4; 1.8 1.8 1.8 1.8]];
  trajectory = [trajectory [(14:-1:2)/10;0.4*ones(1,13);ones(1,13)*1.8]];
  trajectory = [trajectory [(1:-1:-10)/10;-0.4*ones(1,12);ones(1,12)*1.8]];
  %trajectory = [trajectory [(15:20)/10;-ones(1,11)*0.4;ones(1,11)*1.8]];
  data = simulated_data(tb,trajectory, 0.1);
  
  generator = noise_generator(10);
  ev = run(generator,data);

  algorithm = boyen_koller('linearize_with_prior',0);
  model = slat_model(ev.data.testbed, 'obs_var',100, ...
                     'assumed_clusters',{[1 2],[2 3 4]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==6
  % An experiment similar to #4, with more cameras
  pos=[0.75*(-2:3); ...
       zeros(1,6); ...
       ones(1,6)*3.5;
       ones(1,6)*pi/2;
       ones(1,6)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  problem=generateSlatProblem(calib,'range',[-3 3 -2 2],'fps',4);
  ev = importSimulatedProblem(problem, 'align6');
  algorithm = boyen_koller;
  model = slat_model(ev.data.testbed, 'obs_var',100, ...
                     'assumed_clusters',{[1 2 3 4],[5 6]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==7
  % A line of cameras
  pos=[0.75*(-3:3); ...
       0.25*(-ones(1,7)).^(1:7); ...
       ones(1,7)*3.5;
       ones(1,7)*pi/2;
       ones(1,7)*(-pi/2)];
  calib=generateCalibration(KK, pos);
  problem=generateSlatProblem(calib,'range',[-3 3 -2 2],'fps',4);
  ev = importSimulatedProblem(problem, 'align7');
  algorithm = boyen_koller;
  model = slat_model(ev.data.testbed, 'obs_var',100, ...
      'assumed_clusters',{[1 2] [2 3] [3 4] [4 5] [5 6] [6 7]});
  ex = slat_experiment(ev, algorithm, model);
elseif id==8
  tb=slat_testbed('271ff92e-ecf9-1028-84ec-000d56d69dd5');
  dat=generate_data(tb);
  ev=run(noise_generator(3),dat);
  algorithm=boyen_koller;
  model=slat_model(ev.data.testbed,'obs_var',9,...
      'assumed_clusters',triangulate(generateSlatAdjacency(12,1)));
  ex = slat_experiment(ev,algorithm,model);
end

