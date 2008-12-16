function [dm,likelihoods,required] = load_decomposable(model, n, u)
% loads a model as a decomposable model

import prl.*;

[priors,likelihoods,required] = load_flat(model, n, u);
dm = decomposable;
dm.multiply_in(priors);
