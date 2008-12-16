function y = loadfix(filename, ncol)
if ~exist('ncol','var')
  ncol = 1;
end

y = load(filename);
if isempty(y)
  y = zeros(0,max(ncol,size(y,2)));
end
