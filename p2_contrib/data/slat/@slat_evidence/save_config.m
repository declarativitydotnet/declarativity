function save_config(ev, fid)
assert(isscalar(ev));

out = lisp_ostream(fid);
s = get(ev);

print_setf(out, 'slat-time', s.time);
print_setf(out, 'slat-dt', s.time(2)-s.time(1));
print_setf(out, 'slat-obs', s.obs);
print_setf(out, 'slat-visible', logical(s.visible));
print_setf(out, 'num-steps', int32(nsteps(ev)));
print_setf(out, 'num-cams', int32(ncams(s.data.testbed)));

print_setf(out, '*marker-height*', s.data.height);

% THIS NEEDS TO BE FIXED
print_setf(out, 'slat-pos', s.data.trajectory(:,1));

