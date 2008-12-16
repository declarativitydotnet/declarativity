function v = isconnected(a, skip)
if isempty(a)
  v = true; return
end

n = length(a);
if isvector(a)
  i = find(a);
  sa = sparse([i a(i)], [a(i) i], 1, n, n);
else
  sa = sparse(a);
end

if exist('skip','var')
  i = setdiff(1:n, skip);
  sa = sa(i, i);
end

d = bfs(sa,1);
v = all(d>=0);
