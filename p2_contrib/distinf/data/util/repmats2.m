function [c,d] = repmats2(a,b)
nd = max(ndims(a),ndims(b));
sizea = ones(1,nd); sizea(1:ndims(a))=size(a);
sizeb = ones(1,nd); sizeb(1:ndims(b))=size(b);
sizec = max(sizea,sizeb);
if ~all(sizea==sizec | sizea==1) || ~all(sizeb==sizec | sizeb==1)
  error('Incompatible dimensions');
else
  c = repmat(a, sizec./sizea);
  d = repmat(b, sizec./sizeb);
end
