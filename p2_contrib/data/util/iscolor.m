function val=iscolor(str)
val=false(size(str));
for i=1:length(str)
  val(i) = any(lower(str(i))=='bgrcmyk');
end
