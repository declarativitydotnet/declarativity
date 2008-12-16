function adjacency=adjacency_mn(nCams, nNeighbors, wrap)
adjacency=eye(nCams);
eyen=adjacency;
if ~exist('wrap', 'var')
  wrap=0;
end
for i=1:nNeighbors,
  if wrap
    adjacency=adjacency+[eyen(:,i+1:end) eyen(:,1:i)]+...
        [eyen(:,end-i+1:end) eyen(:,1:end-i)];
  else
    adjacency=adjacency+diag(ones(1,nCams-i),i)+diag(ones(1,nCams-i),-i);
  end
end
