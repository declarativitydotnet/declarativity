function adj=sparse_grid(m, n)
% generates a sparse markov network for a grid
% m - x direction
% n - y drection

nnodes = m*n;
adj=eye(nnodes);

% a(i,j) = the camera at the coords (i,j)
a = reshape(1:(m*n),n,m)';

% First the strings of cameras on the sides
for i=1:(m-1):m,
  for j=2:n,
    v = a(i,j);
    w = a(i,j-1);
    adj(v,w)=1;
    adj(w,v)=1;
  end
end

% Now connect the strings of cameras from left to right
for i=1:(m-1)
  for j=1:n
    v=a(i,j);
    w=a(i+1,j);
    adj(v,w)=1;
    adj(w,v)=1;
  end
end

%for i=0:nnodes-1,
%  for j=i+1:nnodes-1,
%    if adj(i+1,j+1)==1,
%      line([floor(i/n);floor(j/n)],[mod(i,n);mod(j,n)]);
%    end
%  end
%  
%end
