function vars = assign_vars(m, n, nnodes)

v = reshape(1:m*n, m, n);

a=round(sqrt(nnodes));
assert(nnodes == a^2);
assert(m>=a && n>=a);

ms = linspace(0, m, a+1);
ns = linspace(0, n, a+1);

k=1;
for j=1:a
  for i=1:a
    vars{k} = v(floor(ms(i)+1):floor(ms(i+1)), floor(ns(j)+1):floor(ns(j+1)));
    k=k+1;
  end
end
