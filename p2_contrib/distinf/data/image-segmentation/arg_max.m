function  a = arg_max(bel)
a = prl.assignment;
for i=1:length(bel)
  a.insert(bel(i).arg_max);
end
