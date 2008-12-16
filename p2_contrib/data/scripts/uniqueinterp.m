function yi = uniqueinterp(x, y, xi)
if length(x)<2
  yi=zeros(size(xi));
else
  [x,i] = unique(x);
  y = y(i);
  yi = interp1(x, y, xi, 'nearest','extrap');
end

