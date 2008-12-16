function object=slat_evidence(arg, algorithm, visible, obs, time)
% slat_evidence is a sequence of observations at regular time intervals
%
% slat_evidence(data, algorithm, visible, obs, time)
%

if nargin==0,                           % default constructor
  bdo = basic_sql_object;
  object = class(struct([]), 'slat_evidence', bdo);
  
elseif nargin==1 && isa(arg, 'slat_evidence') % copy constructor
  object = arg;
  
else

  if ischar(arg) || isstruct(arg)       % id or values
    bdo = basic_sql_object('slat_evidence', arg);
  else
    values = struct('data', arg, 'algorithm', algorithm, ...
        'visible', visible, 'obs', obs, 'time', time);
    bdo = basic_sql_object('slat_evidence', values);
  end
  
  object = class(struct([]), 'slat_evidence', bdo);
end
