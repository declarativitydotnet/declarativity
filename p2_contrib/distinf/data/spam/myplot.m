function h=myplot(a,varargin)

switch size(a,2)
  case 2
    h = plot(a(:,1),a(:,2),varargin{:});
  case 3
    h = plot3(a(:,1),a(:,2),a(:,3),varargin{:});
  otherwise
    error('Invalid dimensions');
end
