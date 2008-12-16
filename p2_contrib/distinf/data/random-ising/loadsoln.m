function s=loadsoln(algorithm,varargin)
root=getenv('EXPROOT');
assert(~isempty(root));
s=loadbpsoln([root filesep 'distinf/random-ising'], algorithm, varargin{:});
