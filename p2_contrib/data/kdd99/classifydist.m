function [yh,mind]=classifydist(x, c, labels, scale)

m = size(x, 1);
n = size(c, 1);

if nargin>3
  x = x ./ repmat(scale, m, 1);
  c = c ./ repmat(scale, n, 1);
end

d = eucldist(x, c);
[mind, cluster] = min(d, [], 2);
yh = labels(cluster);
