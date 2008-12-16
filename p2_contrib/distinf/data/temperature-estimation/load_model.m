function [factors, likelihoods] = load_model(filename, n)
% function [factors, likelihoods] = load_model(filename, n)
% loads a flat representation of the network

load(filename);

import prl.*;
t = tempvars(n);
tlam = model.tlambda(1:n,1:n);
tvec = tlam*model.tmean(1:n);

nf = 0;

[is, js, ws] = find(tlam);

for k=1:length(is)
  i=is(k);
  j=js(k);
  w=ws(k);
  if i==j
    factors(nf+1) = canonical_gaussian(vec(t(i)), w, tvec(i)); nf = nf+1;
  elseif i<j
    factors(nf+1) = canonical_gaussian(t([i j]), [0 w; w 0], zeros(2,1)); nf = nf+1;
  end
end 

likelihoods = [];
