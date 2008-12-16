function s=loadjtsoln(directory, algorithm, pattern, varargin)

probnames = findprobs(pattern);

for k=1:length(probnames)
  disp(probnames{k});
  name = [directory filesep algorithm filesep probnames{k}];
  s.name = strrep([algorithm filesep probnames{k}],filesep,'/');

  s = loadparams(name, varargin{:});
  
  % Load the junction tree info
  s = loadrst(name, s, 5);
  [s.c,tc] = loadcliques([name '.db'], s.n, s.epoch);
  s.ec = tc / s.epoch;
  
  % the loading process is a little broken - we should be checking
  % triangulated also when the spanning tree changes
  c=s.c;
  for i=1:size(c,1)
    j = find(s.ea<=s.ec(i), 1, 'last');
    if isempty(j) || ~isconnected(s.a{j},5) % we don't even have a spanning tree yet
      s.triangulated(i) = nan;
      %s.minimal(i,1) = 0;
    else
      cg = make_cluster_graph(s.a{j}, c(i,:));
      if s.n>=5
        cg.remove_vertex(5); % annoying #5
      end
      s.triangulated(i) = cg.tree && cg.running_intersection;
    end
  end
  
  % Load the beliefs
  if has_table([name '.db'], 'beliefs')
    s.bel = loadbeliefs([name '.db'], s.n, s.t);
    s = compute_stats(probnames{k}, s);
  end

    
%   
%   and compute some belief stats
%   beliefs = load([name '-beliefs.txt']);
%   if ~isempty(beliefs)
%     load(probnames{k}); % load the problem
%     if isfield(model, 'bias')
%       s.bias  = interp_nodes(beliefs, s.t);
%       bias = model.bias(1:size(s.bias,1));
%       s.rms = sqrt(sum((s.bias-repmat(bias, 1, length(s.t))).^2, 1));
%     else
%       s.temp  = interp_nodes(beliefs, s.t);
%       temp = model.tmean(1:size(s.temp,1));
%       s.rms = sqrt(sum((s.temp-repmat(temp, 1, length(s.t))).^2, 1));
%     end
%   end
  
  % Load the bandwidth info
  s = loadbandwidth(name, s); 

end
