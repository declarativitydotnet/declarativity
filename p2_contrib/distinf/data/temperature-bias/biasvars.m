function b=biasvars(n)
global u;
for i=1:n,
  b(i) = u.new_vector_variable(['b' num2str(i)], 1);
end
