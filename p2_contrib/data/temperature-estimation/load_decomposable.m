function [dm,t] = load_decomposable(model, n, u)
% loads a model as a decomposable model

import prl.*;

[factors, t] = load_flat(model, n, u);
dm = decomposable;
dm.multiply_in(factors);
