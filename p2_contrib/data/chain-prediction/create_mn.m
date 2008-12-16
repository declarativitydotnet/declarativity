function [mn,v]=create_mn(name,nodes)
import prl.*
load(name)
global u
if isempty(u) 
  u = universe;
end

if exist('nodes','var')
  Ls=Ls(nodes);
  adjMatrix=adjMatrix(nodes,nodes);
  Psi=Psi(nodes,nodes);
end

mn = markov_network_discrete;

% Create the nodes
for i=1:length(Ls)
  v(i) = u.new_finite_variable(num2str(i), length(Ls{i}));
  mn.add_node(v(i), table_factor(vec(v(i)), Ls{i}));
end

% Create the edges
ind = find(adjMatrix);
for k=1:length(ind)
  [i,j] = ind2sub(size(adjMatrix), ind(k));
  assert(all(size(Psi{i,j}) == [length(Ls{i}) length(Ls{j})]));
  mn.add_edge(v(i), v(j), table_factor(v([i j]), Psi{i,j}(:)));
end

