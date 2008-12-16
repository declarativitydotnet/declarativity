function genising(m, n, c, indices)
dir=sprintf('%dx%d-%d',m,n,c);
[d1,d2]=mkdir(dir);
for i=indices
  model=random_ising(m,n,-0.5*c, 0.5*c);
  save(sprintf('%s/%02d',dir,i),'model');
end

