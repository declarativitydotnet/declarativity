function mn=create_mn(model)
import prl.*;
global u
if isempty(u)
  u = universe;
end

m = model.m;
n = model.n;

[is,js,ws] = find(model.edgelam);

% Create the graphical model
mn = markov_network_discrete;
v = u.new_finite_variables(m*n, 2);

% Add nodes
for i = 1:m*n
  np = model.nodepot(i);
  mn.add_node(v(i), table_factor(vec(v(i)), [np 1-np]));
end

% Add edges
for i = 1:length(is)
  if is(i) < js(i)
    v1 = v(is(i));
    v2 = v(js(i));
    ew = exp(ws(i));
    mn.add_edge(v1, v2, table_factor([v1 v2], [ew 1/ew 1/ew ew]));
  end
end

