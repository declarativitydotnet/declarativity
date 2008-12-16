function save_config(model, fid)
assert(isscalar(model));
s = get(model);
out = lisp_ostream(fid);

% for i=1:length(s.assumed_clusters)
%   clusters{i} = s.assumed_clusters{i}-1;
% end

fprintf(fid, '(setf *parametrization* :%s)\n', s. parameterization);
print_setf(out, '*motion-model*', [':' s.motion_model]);
print_setf(out, '*velocity*', s.velocity);
print_setf(out, '*acceleration*', s.acceleration);
print_setf(out, '*estimate-pan*', logical(s.estimate_pan));
print_setf(out, '*estimate-tilt*', logical(s.estimate_tilt));
print_setf(out, '*estimate-height*', logical(s.estimate_height));
print_setf(out, '*accurate-pan-prior*', logical(0));
print_setf(out, '*tilt-var*', s.tilt_var);
print_setf(out, '*height-var*', s.height_var);
print_setf(out, '*obs-var*', s.obs_var);
fprintf(fid, '(setf *assumed-density* :%s)\n', s.assumed_density);
print_setf(out, '*assumed-clusters*', ['(list ' ...
      iset(s.assumed_clusters{:}) ')']);

fprintf(fid, '(setf slat-cams `#(');
for k=1:length(s.calib)
  calib = s.calib(k);
  fprintf(fid, ',(make-perspective-camera ');
  print(out, calib.pos);
  print(out, [calib.pan; calib.tilt; 0]);
  print(out, calib.KK);
  print(out, calib.imageSize');
  fprintf(fid, ') ');
end
fprintf(fid, '))\n');

% Unsupported/deprecated features:
% (setf *pan-components* 8)  ; # of mixture components for pan

  
    
