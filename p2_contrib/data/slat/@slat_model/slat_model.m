function object=slat_model(arg1, varargin)
% slat_model captures all the model parameters for SLAT
%
% slat_model(values)
% slat_model(testbed, 'property', value, ...)
% slat_model(calib, 'property', value, ...)
%

if nargin==0,                           % default constructor
  so=struct_object;
  object = class(struct([]), 'slat_model', so);
  
elseif nargin==1 && isa(arg1, 'slat_model') % copy constructor
  object = arg1;
  
else
  
  if nargin==1 && isstruct(arg1) && length(arg1)==1 % struct construct
    values = arg1;
    if ~isfield(values, 'motion_model')
      values.motion_model = 'deprecated';
      values.velocity = 0;
    end
    so = struct_object(values);
  else                                  % standard constructor
    if isa(arg1, 'slat_testbed')
      values.calib = arg1.calib;
    elseif isstruct(arg1)
      values.calib = arg1;
    else
      error('Invalid arguments');
    end
      
    values = process_options(fields(slat_model, 0), values, varargin);
    so     = struct_object(values);
  end

  object = class(struct([]), 'slat_model', so);

end

