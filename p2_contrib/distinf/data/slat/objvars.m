function vars = objvars(model)
% function vars = objvars(model)
% Returns the state variables associated with an object
global u; % The global universe
vars = [u.vector_timed_process('l', 2).current, ...
        u.vector_timed_process('v', 2).current];
