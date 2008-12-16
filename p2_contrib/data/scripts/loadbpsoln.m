function s=loadsoln(directory,algorithm,varargin)
  
if strcmp(algorithm(1:2),'bp')
  s = loadolgsoln(directory, algorithm, varargin{:});
else
  s = loadcppsoln([directory filesep algorithm], varargin{:});
end



