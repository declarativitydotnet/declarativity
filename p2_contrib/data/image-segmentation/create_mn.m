function mn=create_mn(img)
import prl.*;
global u
if isempty(u)
  u = universe;
end
m = img.rows;
n = img.cols;

[is,js] = find(grid_adjacency(m,n));

% Create the graphical model
mn = markov_network_discrete;
v = u.new_finite_variables(m*n, 2);

% Add nodes
p0 = exp(-(img.data(:)-img.mu(1)).^2/img.sigma(1))';
p1 = exp(-(img.data(:)-img.mu(2)).^2/img.sigma(2))';

for i = 1:m*n
  mn.add_node(v(i), table_factor(vec(v(i)), [p0(i) p1(i)]));
end

w = exp(-2);

% Add edges
for i = 1:length(is)
  if is(i) < js(i)
    v1 = v(is(i));
    v2 = v(js(i));
    mn.add_edge(v1, v2, table_factor([v1 v2], [1 w w 1]));
  end
end

