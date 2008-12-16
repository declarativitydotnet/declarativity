function s=addfields(s,varargin)
assert(mod(length(varargin),2)==0);

for i=1:2:length(varargin)
  if ~isfield(s,varargin{i})
    s.(varargin{i})=varargin{i+1};
  end
end
