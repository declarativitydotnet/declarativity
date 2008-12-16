function soln = loadolgsoln(directory, algorithm, pattern, varargin)
%function soln = load_results(filename)
probnames = findprobs(pattern);

for k=1:length(probnames)

  name = [directory filesep algorithm filesep probnames{k}];
  s.name = strrep([algorithm filesep probnames{k}],filesep,'/');
  
  s = loadparams(name, varargin{:});
  
  % Load the beliefs
  s.beliefs = loadbeliefs(name, s.t);

  % Load the other b.p. stats
  s.normalizer = binned_max(load([name '-normalizer.txt']), s.t);
  s.residual = binned_max(load([name '-residuals.txt']), s.t);
  updates = load([name '-updates.txt']);
  if size(updates,2)==2
    updates=[updates(:,1) ones(size(updates,1),1) updates(:,2)];
  end
  updates = sortrows(updates, 3);
  s.nupdates = uniqueinterp(updates(:,3), cumsum(updates(:,2)), s.t);
  
  % Load the spanning tree info
  if exist([name '-parents.txt'],'file')
    s = loadrst(name, s);
  end  
  
  % Load the bandwidth info
  s = loadbandwidth(name, s); 
  %msgcounts = load([name '-msgcounts.txt']);
  soln(k)=s;
end

function y=mymin(x)
i = x>7.3e5;
y=min(x(i));

function y=mymax(x)
i = x>7.3e5;
y=max(x(i));

function y=binned_max(table, t)
if isempty(table)
  y=[]; return
end
y = zeros(size(t));
t = [0 t];
prev = nan;
for i=1:length(t)-1
  j = find(table(:,3)>t(i) & table(:,3)<=t(i+1));
  if isempty(j)
    y(i) = prev;
  else
    y(i) = max(table(j,2));
    prev = y(i);
  end
end

