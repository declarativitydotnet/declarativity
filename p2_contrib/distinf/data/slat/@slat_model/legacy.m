function model=legacy(object)
% 
% converts the slat_model object to a legacy Matlab structure
% (for visualization etc.)
% Does _not_ work for models with tilt/height yet.

%(setf *monitor-alignment* nil)          ; monitor the alignment? (costly)

s = get(object);

prob.calib = s.calib;
prob.pos = [0;0;0];
prob.visible = zeros(length(prob.calib),1);

switch s.parameterization
  case 'uv'
    model = generateSlatModel(prob, 3, 0, 2, 0, 1);
  case 'xy'
    model = generateSlatModel(prob, 2, 0, 2, 0, 1);
  otherwise
    error('Unknown parameterization');
end

var = s.acceleration^2;

model.Qdt(1:4,1:4) = diag([0 0 var var]);

%model.calib = s.calib;
