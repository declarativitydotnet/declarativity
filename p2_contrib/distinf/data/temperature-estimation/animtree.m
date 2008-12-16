function animtree(s,loc)
for i=1:length(s.ep)
  plottree(loc, s.p(i,:));
  title(num2str(s.ep(i)));
  pause;
end
