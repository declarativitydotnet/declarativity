function s=loadsoln(algorithm,prob, varargin)
root=getenv('EXPROOT');
assert(~isempty(root));
s=loadjtsoln([root filesep 'distinf/temperature-bias'], algorithm, prob, varargin{:});
