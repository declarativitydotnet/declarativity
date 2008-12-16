function plotmean(varargin)
% plotmean(xvals1, yvals1 [,style1], ...)
% Plots the mean (or other estimate) of yvals* over 2nd and higher
% dimension.  yvals are arranged, so that the first dimension corresponds
% to the x axis.
%
% Options: estimator, plotbars, nbars

defstyle = {'-r', '-.b', '--k', 'm', '^-g', '*-c'};

i = 1;
n = 0;
while i<=length(varargin) && isnumeric(varargin{i})
  n = n +1;
  xvals{n} = varargin{i};
  yvals{n} = varargin{i+1};
  assert(isnumeric(yvals{n}));
  i = i + 2;
  if i<=length(varargin) && ischar(varargin{i}) && ~any(strcmp(varargin{i},{'estimator', 'plotbars', 'nbars', 'conf'}))
    style{n} = varargin{i}; i = i + 1;
  else
    style{n} = defstyle{n};
  end
  color{n} = style{n}(iscolor(style{n}));
end

[estimator, plotbars, nbars, conf] = ...
   process_options(varargin(i:end), ...
     'estimator', 'mean', ...
     'plotbars', 1, ...
     'nbars', 10, ...
     'conf', 2);

held = ishold;

for i=1:n
  assert(length(xvals{i})==size(yvals{i},1));
  for j=1:size(yvals{i},1)
    value = yvals{i}(j,:);
    k = find(~isnan(value));
    estimate{i}(j) = feval(estimator, value(k));
    error{i}(j) = std(value(k))/sqrt(length(k))*conf;
  end
end

handles = [];

for i=1:n
  h = plot(xvals{i}, estimate{i}, style{i});
  set(h,'linewidth',2);
  if i==1
    hold on;
  end
end

if plotbars
  for i=1:n
    bari = 1:max(1,round(length(xvals{i})/nbars)):length(xvals{i});
    h = errorbar(xvals{i}(bari), estimate{i}(bari), error{i}(bari), [color{i} '.']);
    set(h,'linewidth',2);
  end
end

if ~held
  hold off;
end

