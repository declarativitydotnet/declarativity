function saveprob(prob, filebase)
% function saveprob(prob, filebase)
% writes two files filename-cam.txt and filename-obs.txt

calib = prob.calib;
for i=1:length(calib)
  KK = calib(i).KK;
  camdata(i,:) = [calib(i).pos' calib(i).pan calib(i).tilt KK(1,1) KK(2,2) KK(1:2,3)'];
end
f = fopen([filebase '-cam.txt'], 'wt');
fprintf(f, '%f %f %f %f %f %f %f %f %f\n', camdata');
fclose(f);

[i,j] = find(prob.visible);
for k=1:length(i)
  obsdata(k,:) = [j(k) i(k) prob.obs{i(k)}(:,j(k))'];
end
f = fopen([filebase '-obs.txt'], 'wt');
fprintf(f, '%d %d %3.8f %3.8f\n', obsdata');
fclose(f);
