function mat = ocell2mat(array)
% mat = ocell2mat(cell)
% Converts a cell array of objects (in which eahc entry has exactly
% 1 object) into a corresponding object.  Empty entries are
% converted to the value returned by the default constructor for
% the class.  If the cell array contains objects of different
% kinds, the output matrix will have the type of the first cell.

type = [];

% First, determine the type
for i=1:numel(array)
  if ~isempty(array{i})
    type = class(array{i});
    break;
  end
end

if isempty(type)
  error('No objects found');
end

mat(1:numel(array)) = feval(type);
mat = reshape(mat, size(array));

for i=1:numel(array)
  mat(i) = array{i};
end

