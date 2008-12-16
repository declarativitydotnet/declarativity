function vars = statevars(model)
vars = [posevars(model); prl.domain(objvars(model))];

