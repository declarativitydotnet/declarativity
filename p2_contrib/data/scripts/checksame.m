function checksame(varargin)
for i=2:length(varargin)
  assert(all(varargin{1}(:)==varargin{i}(:)));
end
