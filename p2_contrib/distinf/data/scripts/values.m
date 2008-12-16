function val = values(f,j)

val = zeros(1,size(f,1));
for i=1:length(f)
  v = f(i).values;
  val(i) = v(j);
end
