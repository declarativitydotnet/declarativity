function names=findprobs(pattern)
files=dir([pattern '*.mat']);
if isempty(files)
  error(['cannot file problems that match ' pattern]);
end
path=fileparts(pattern);
if isempty(path)
  path = '.';
end
for i=1:length(files)
  [dummy,name]=fileparts(files(i).name);
  names{i} = [path filesep name];
end
