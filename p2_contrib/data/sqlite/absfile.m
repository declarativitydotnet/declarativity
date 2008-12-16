function absname = fullname(filename)
% name = fullname(filename)
% Converts a relative filename to an absolute one
  
[path,name,ext,version] = fileparts(filename);
if length(path)>=2 && (path(1) =='/' || path(2) ==':')
  absname = filename;
else
  absname = fullfile(pwd, path, [name ext version]);
end

