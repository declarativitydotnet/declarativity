function value=ispartition(partition, numbers)
% function value=ispartition(partition, numbers)
% returns true iff cellarray forms a partition of the set <numbers>
%

elements = [];
for i=1:length(partition)
  if ~isempty(intersect(elements, partition{i}))
    value = 0;
    return;
  end
  elements = union(elements, partition{i});
end

value = isempty(setxor(elements, numbers));
