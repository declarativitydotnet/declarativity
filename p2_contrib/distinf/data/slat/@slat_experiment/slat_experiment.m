function object=slat_experiment(arg, algorithm, varargin)
% slat_experiment desribes precise settings, under which a solution is computed
%
% slat_experiment(evidence, algorithm [, model] [, scenario], ...)
% slat_experiment(evidence, algorithm [, scenario] [, model], ...)
% 
% when used with a noise-generated data set and model is not specified,
% plugs in the noise variance from the noise generator

if nargin==0,                           % default constructor
  object = class(struct([]), 'slat_experiment', experiment);
  
elseif nargin==1 && isa(arg, 'slat_experiment') % copy constructor
  object = arg;
  
else
  if ischar(arg) || isstruct(arg)       % id or values
    exp = experiment(arg);
  else                                  % standard constructor
    % Process the arguments, plugging in defaults for the missing values
    evidence = arg;
    scenario = convergence_scenario(1:ncams(testbed(evidence)));
    model = slat_model(testbed(evidence));
    if isa(evidence.algorithm, 'noise_generator')
      model = set(model, 'obs_var', evidence.algorithm.sigma_u^2);
    end
    if isa(algorithm, 'rahimi_map')
      % Ali uses the standard XY parameterization
      model = set(model, 'parameterization', 'xy');
    end
    
    i = 1;
    while i<=length(varargin)
      argi = varargin{i};
      if ischar(argi)
        break;
      elseif isa(argi, 'slat_model')
        model = argi;
      elseif isa(argi, 'network_scenario')
        scenario = argi;
      else
        disp(argi);
        error('... invalid argument');
      end
      i = i+1;
    end

    values = struct('model',  model, 'evidence', evidence);
    exp = experiment(values, algorithm, scenario);
  end
  
  object = class(struct([]), 'slat_experiment', exp);
end
