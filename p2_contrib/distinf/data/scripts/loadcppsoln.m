function soln=loadsoln(algorithm, pattern)
% function soln=loadsoln(algorithm, pattern)
probnames = findprobs(pattern);
for i=1:length(probnames)
  name = [algorithm filesep probnames{i}];
  
  % load the beliefs
  if exist([name '-beliefs.txt'],'file')
    bel = load([name '-beliefs.txt']);
  else
    bel = [];
  end
  if ~isempty(bel) 
    nvars = find(bel(1,:)==-1,1) - 1;
    nrecords = size(bel,1);
    ind = bel(1,1:nvars);
    assert(~isempty(nvars));
    assert(all(all(bel(:,1:nvars)==repmat(ind, nrecords,1))));
    [dummy,ordered] = sort(ind);
    bel = bel(:,nvars+3:2:end);
    soln(i).beliefs = bel(:,ordered);
  end
  
  % load the statistics
  res = load([name '-statistics.txt']);
  soln(i).i = res(:,1)';
  soln(i).time = res(:,2)';
  soln(i).nupdates = res(:,3)';
  soln(i).residual = res(:,4)';
  if size(res,2)>4
    soln(i).expresidual = res(:,5)';
  end
end
