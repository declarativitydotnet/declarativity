function graph = distance_mn(tb, threshold)
%function graph = distance_mn(tb, threshold)
calib = get(tb, 'calib');

epsilon = 1e-6;

graph = zeros(length(calib));

for i=1:length(calib)
  for j=1:length(calib)
    graph(i,j) = norm(calib(i).pos - calib(j).pos)<=threshold+epsilon;
  end
end

    
