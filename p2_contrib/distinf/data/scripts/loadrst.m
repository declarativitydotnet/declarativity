function s=loadrst(name,s, varargin)
  
[s.p,tp] = loadparents([name '-parents.txt'], s.epoch);
s.ep = tp / s.epoch;

[s.a,ta] = loadedges([name '-edges.txt']);
s.ea = ta / s.epoch;

for i=1:size(s.p,1)
  s.connected(i) = isconnected(s.p(i,:), varargin{:});
end
