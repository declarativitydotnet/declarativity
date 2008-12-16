function bel=node_beliefs(ss,set)
v = set.values;
for i=1:length(v)
  bel(i) = ss.belief(prl.domain(v(i)));
end
