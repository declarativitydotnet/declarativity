function t=tempvars(n)
global u; % The global universe
for i=1:n,
  t(i) = u.new_vector_variable(['t' num2str(i)], 1);
end
