function model = gen_ising(m, n, minlam, maxlam)

if ~exist('minlam')
  minlam = 0;
end
if ~exist('maxlam')
  maxlam = 1;
end

ind = reshape(1:m*n, m, n);
model.m = m;
model.n = n;
model.ind = ind;
model.nodepot = rand(m,n);  % p(X_i = 1)
model.edgelam = sparse(m*n,m*n);
for i=1:m,
  for j=1:n,
    if i<m
      model.edgelam(ind(i,j),ind(i+1,j)) = rand*(maxlam-minlam)+minlam;
    end
    if j<n
      model.edgelam(ind(i,j),ind(i,j+1)) = rand*(maxlam-minlam)+minlam;
    end
  end
end

model.edgelam = model.edgelam+model.edgelam';
