function [cg,t] = prlmodel(model, n, u)
import prl.*;
t = tempvars(n, u);
tlam = model.tlambda(1:n,1:n);
tmean= tlam*model.tmean(1:n);
cg = canonical_gaussian(t, tlam, tmean, 1);
