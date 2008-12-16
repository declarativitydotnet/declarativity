function save_gridvars(m, n, vartype, basename, nnodes)
vars = assign_gridvars(m, n, nnodes);
save_vars('', vars, vartype, basename);
