function a=grid_adjacency(m,n)
N = m*n;
varid = reshape(1:m*n, m, n);

a = sparse(N,N);

% Compute the edges
for i=1:m,
  for j=1:n,
    if j<n
      a(varid(i,j), varid(i,j+1)) = 1;
    end
    if i<m
      a(varid(i,j), varid(i+1,j)) = 1;
    end
  end
end
a = a+a';
