function vector=colvector(vector)
% vector=colvector(vector)
% adjusts the vector, so that it's a column vector

assert(isvector(vector));

if size(vector,1)==1
  vector = reshape(vector,[],1);
end

  
