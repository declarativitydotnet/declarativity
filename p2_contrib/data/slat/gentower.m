function ex=generateSlatTower(id)
% function ex=generateSlatTower(id)
% Generates a "Tower" scenario from te IPSN paper
% this script isn't working now
if id==1
  tb=testbed('271ff92e-ecf9-1028-84ec-000d56d69dd5');
  dat=generate_data(tb);
  ev=run(noise_generator(3),dat);
  algorithm=boyen_koller;
  model=slat_model(ev.data.testbed,'obs_var',9,...
      'assumed_clusters',triangulate(generateSlatAdjacency(12,1)));
  ex = experiment(ev,algorithm,model);
elseif id==2
  tb=testbed('271ff92e-ecf9-1028-84ec-000d56d69dd5');
  dat=generate_data(tb,'mps',0.5);
  ev=run(noise_generator(15),dat);
  algorithm=boyen_koller;
  model=slat_model(ev.data.testbed,'obs_var',15^2,...
      'assumed_clusters',triangulate(generateSlatAdjacency(12,1)));
  ex = experiment(ev,algorithm,model);
end

