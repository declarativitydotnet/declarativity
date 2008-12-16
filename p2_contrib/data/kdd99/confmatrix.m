function [m,precision,recall]=confmatrix(y,yh)
% x=confmatrix(yt,yh)
% returns the confusion matrix for the given ground truth and prediction
y=y+1;
yh=yh+1;
k=max(max(y),max(yh));
m=zeros(k,k);
for i=1:k, % ground truth
  yhi=yh(y==i);
  for j=1:k % prediction
    m(i,j)=sum(yhi==j);
  end
end
precision = diag(m)'./sum(m,1);
recall = diag(m)./sum(m,2);
