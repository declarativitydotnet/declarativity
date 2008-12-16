function s=loadsoln(algorithm, varargin)
root=getenv('EXPROOT');
assert(~isempty(root));
s=loadbpsoln([root filesep 'distinf/image-segmentation'], algorithm, varargin{:});
