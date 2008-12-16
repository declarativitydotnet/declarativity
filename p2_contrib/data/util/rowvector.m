function vector=rowvector(vector)
% vector=rowvector(vector)
% adjusts the vector, so that it's a row vector

assert(isvector(vector));

if size(vector,2)==1
  vector = reshape(vector,1,[]);
end

  
