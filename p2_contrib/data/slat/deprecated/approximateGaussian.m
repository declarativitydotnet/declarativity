function [muY, sigmaY, valid] = approximateGaussian(muX, sigmaX, fn, Q, param)
% [muY, sigmaY, valid] = approximateGaussian(muX, sigmaX, fn, Q, param)
% Computes an approximation of the normal distribution of Y=fn(X)+N(0,Q)
% where fn is a deterministic function of X.
% 
% The function fn must take as a parameter a matrix XX, where each column
% is one instantiation of the variable X. Param, if specified, is
% passed to the function fn.

% This function implements precision 5 monomial method


d = length(muX);
u=sqrt(3);
w0 = 1+(d^2 - 7*d)/18;
w1 = (4-d)/18;
w2 = 1/36;

p0 = zeros(d,1);
p1 = u*[eye(d) -eye(d)];
p2 = zeros(d,0);

for i=1:d-1,
  %i is the index of the first non-zero coordinate
  pts = u*[zeros(i-1, 4*(d-i));
    ones(1,2*(d-i)) -ones(1,2*(d-i));
    eye(d-i) -eye(d-i) eye(d-i) -eye(d-i)];
  p2 = [p2 pts];
end

%A = sqrtm(sigmaX);
A = chol(sigmaX)';
[stuff,np1] = size(p1);
[stuff,np2] = size(p2);
p0 = A*p0 + muX;
p1 = A*p1 + muX*ones(1,np1);
p2 = A*p2 + muX*ones(1,np2);

f0=fn(p0,param(1));
dy=length(f0);

muY=zeros(dy,length(param));
sigmaY=zeros(dy,dy,length(param));
valid=zeros(1,length(param));

for k=1:length(param)
  [f0, valid0] = fn(p0, param(k));
  if valid0
    [f1, valid1] = fn(p1, param(k));
    if all(valid1)
      [f2, valid2] = fn(p2, param(k));
      if all(valid2)
        muY(:,k) = w0*f0 + w1*sum(f1,2) + w2*sum(f2,2);
        
        sigmaY1 = w0*f0*f0';
        for i=1:np1,
          sigmaY1 = sigmaY1 + w1*f1(:,i)*f1(:,i)';
        end
        for i=1:np2,
          sigmaY1 = sigmaY1 + w2*f2(:,i)*f2(:,i)';
        end
        sigmaY1 = sigmaY1 - muY(:,k)*muY(:,k)';
        sigmaY1 = sigmaY1+Q;
        sigmaY(:,:,k) = (sigmaY1+sigmaY1')/2;
        valid(k)=1;
      end
    end
  end
end
