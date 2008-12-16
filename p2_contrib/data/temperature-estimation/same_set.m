function same=same_set(a,b)
if iscell(a)
  assert(all(size(a)==size(b)));
  same = zeros(size(a));
  for i=1:numel(a)
    same(i) = length(a{i})==length(b{i}) && all(sort(a{i})==sort(b{i}));
  end
else
  same = length(a)==length(b) && all(sort(a)==sort(b));
end
