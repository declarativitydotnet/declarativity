function [xt,Pt] = kf(x0, P0, A, Q, H, R, y)
% The model is as follows:
% x(t) = Ax(t-1) + N(0,Q)
% y(t) = Hx(t-1) + N(0,R)

xt(:,1)=x0;
Pt(:,:,1)=P0;

x=x0;
P=P0;
n=length(x0);

dimy=size(y);

for t=1:dimy(2),
  % Prediction 
  x=A*x;
  P=A*P*A'+Q;
  
  % Measurement update
  K=P*H'*inv(H*P*H'+R);  %gain
  x=x+K*(y(:,t)-H*x);
  P=(eye(n)-K*H)*P;
  
  % Store the result
  xt(:,t+1)=x;
  Pt(:,:,t+1)=P;
  
end
