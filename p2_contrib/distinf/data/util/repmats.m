function varargout = repmats(varargin)
maxdims = 2;
for i=1:length(varargin)
  maxdims = max(maxdims, ndims(varargin{i}));
end

dims = ones(length(varargin),maxdims);
for i=1:length(varargin)
  ndimsi = ndims(varargin{i});
  dims(i,1:ndimsi) = size(varargin{i});
end

dim = max(dims,[],1);
for i=1:length(varargin);
  if ~all(dims(i,:)==dim | dims(i,:)==1)
    error('Incompatible dimensions');
  end
  varargout{i} = repmat(varargin{i}, dim./dims(i,:));
end
