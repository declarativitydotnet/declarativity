function d=eucldist(x, y)
% eucldist(x, y)
% returns a matrix of euclidean distances between two datasets x and y

m = size(x,1);
n = size(y,1);
d = zeros(m,n);

for i=1:n
  d(:,i) = sqrt(sum((x - y(ones(1,m)*i,:)).^2, 2));
end
