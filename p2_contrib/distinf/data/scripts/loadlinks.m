function a=loadlinks(nnodes, commtype)

if nargin<2
  commtype='chain';
end

switch commtype
  case 'chain'
    a = diag(ones(1,nnodes-1),1); a = a+a';
  case 'berkeley'
    load ../temperature-estimation/link_quality
    i = setdiff(1:54, 5);
    a = a(i,i);
    a = a(1:nnodes, 1:nnodes);
  otherwise
    error('Unknown communication pattern.');
end
