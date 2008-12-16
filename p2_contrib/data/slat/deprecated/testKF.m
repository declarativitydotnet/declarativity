x0 = [0;0];
P0 = eye(2);
A = [1 0.01; 0 1];
Q = 0.01*eye(2);
H = [1 0];
R = 4;

n=3000;

x = sqrtm(Q)*randn(2, n);
x(2,:) = cumsum(x(2,:),2);
x(1,:) = cumsum(x(1,:)+0.01*x(2,:),2);
x = [x0 x];

y = H*x(:,2:end)+randn(1,n)*sqrt(R);

figure(1);plot(y,'y');
hold on;
plot(x');

[xt,Pt]=kf(x0, P0, A, Q, H, R, y);

plot(xt(1,:),'--k');
plot(xt(2,:),'--m');

index = 1:n/15:n;
errorbar(index, xt(1,index), 2*sqrt(Pt(1,1,index)), '.k');
errorbar(index, xt(2,index), 2*sqrt(Pt(2,2,index)), '.m');


hold off;
legend('y','x1','x2','xt1','xt2');

