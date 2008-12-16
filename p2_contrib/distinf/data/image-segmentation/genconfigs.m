function genconfigs(pattern, nnodes, varargin)

if nargin>1
  a = loadlinks(nnodes, varargin{:});
end
      
probnames = findprobs(pattern);
for i=1:length(probnames)
  name = probnames{i};
  load(name);
  save_imagecpp(img, name);
  if nargin>1
    save_imagecsv(img, name);
    save_gridvars(img.rows, img.cols, 'F,2', name, nnodes);
    save_links(a, name, nnodes);
  end
end
