function f=fields(object, include_parents)
f = field([]);
f = add(f, field('calib'));
f = add(f, field('parameterization', {'xy', 'uv'}, 'uv', 1));
f = add(f, field('motion_model', {'deprecated' 'additive'}, 'additive', 1));
f = add(f, field('velocity', 'double', 1, 1));
f = add(f, field('acceleration', 'double', sqrt(sqrt(0.3)), 1));
f = add(f, field('estimate_pan', 'logical', 1, 1));
f = add(f, field('estimate_tilt', 'logical', 0, 1));
f = add(f, field('estimate_height', 'logical', 0, 1));
f = add(f, field('pan_var', 'double', 0.01, 1));  % unused
f = add(f, field('tilt_var', 'double', 0.01, 1));
f = add(f, field('height_var', 'double', 0.01, 1));
f = add(f, field('obs_var', 'double', 100, 1));
f = add(f, field('assumed_density', {'normal' 'discrete' 'hybrid'}, ...
                 'normal', 1));
f = add(f, field('assumed_clusters', [], @dense_jt, 1)); 
% modifiable but need a separate set method or extend to check consistency

%%f = add(f, field('assumed_cpc', 'int', @)); 
%% assumed_cpc: the max. number of cameras in the assumed junction tree


function value=dense_jt(values)
value={1:length(values.calib)};


