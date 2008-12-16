function dom=nodevars(n)
global u;
for i=1:n,
  t = u.new_vector_variable(['t' num2str(i)], 1);
  b = u.new_vector_variable(['b' num2str(i)], 1);
  dom(i) = prl.prl.make_domain(t, b);
end
