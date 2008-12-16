function A = mdiag(varargin)
ts=0;
for i=1:length(varargin)
  ai = length(varargin{i});
  assert(all(size(varargin{i})==[ai ai]));
  ts = ts+ai;
end

A = zeros(ts);
ts = 0;
for i=1:length(varargin)
  ai = length(varargin{i});
  A((1:ai)+ts,(1:ai)+ts) = varargin{i};
  ts = ts+ai;
end
