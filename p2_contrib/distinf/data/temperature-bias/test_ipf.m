function [s,sipf] = test_ipf(d, bias, nhops, structure)
import prl.*;

n = length(d);

switch structure
  case 'mst'
    args = argumentsf(d);
    jt=junction_tree(args);
    jt.triangulate;
    a=adjacency(jt);

    for i=1:n
      clique(i) = jt.clique(i);
    end

    cg = make_cluster_graph(a, clique, d);
    jt = factor_junction_tree(cg);
    
  case 'line'
    a = diag(ones(1,n-1),1); a = a+a';
    cg = make_cluster_graph(a, argumentsf(d), d);
    jt = factor_junction_tree(cg, 1);
    
  otherwise
    error('Invalid structure parameter: select line or mst');
end

tv = tempvars(n);
bv = biasvars(n);

% For each vertex, obtain a subgraph 
for i=1:n
  subtree = jt.subtree(i, nhops);
  
  % First, compute the answer using the marginals in the subgraph
  ss = shafer_shenoy(subtree);
  ss.calibrate;
  s.temp(i) = ss.belief(domain(tv(i))).as_canonical_gaussian.as_moment_gaussian.mean;
  s.bias(i) = ss.belief(domain(bv(i))).as_canonical_gaussian.as_moment_gaussian.mean;
  
  % Run IPF
  frag = prodf(subtree.vertex_properties);
  frag = frag.as_fragment_gaussian;
  priors = frag.priors;
  likelihoods = frag.likelihoods;
  ipf = jt_ipf(priors);
  niters = 0;
  while ipf.iterate(1) > 1e-3 && niters < 100; niters = niters + 1; end
  dm = ipf.result;
  dm.multiply_in(likelihoods);
  sipf.temp(i) = dm.marginal(domain(tv(i))).as_moment_gaussian.mean;
  sipf.bias(i) = dm.marginal(domain(bv(i))).as_moment_gaussian.mean;
  sipf.niters(i) = niters;
end

s.rms = rms(bias, s.bias);
sipf.rms = rms(bias, sipf.bias);
