function a=adjacency(graph)
e = graph.edges;
n = graph.num_vertices;
a = sparse(n,n);

for i=1:length(e)
  a(e(i).target, e(i).source)=1;
  a(e(i).source, e(i).target)=1;
end
