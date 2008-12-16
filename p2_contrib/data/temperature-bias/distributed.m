function ss=distributed(adjacency, cliques, factors)
% simulates a distributed experiment with the given cliques
cg = make_cluster_graph(adjacency, cliques, factors);
ss = shafer_shenoy(factor_junction_tree(cg));
ss.calibrate;
