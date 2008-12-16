function s=loadsoln(algorithm,varargin)
root=getenv('EXPROOT');
assert(~isempty(root));
s=loadjtsoln([root filesep 'distinf/temperature-estimation'], algorithm, varargin{:});
