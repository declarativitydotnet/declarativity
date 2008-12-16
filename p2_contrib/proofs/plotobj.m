function [r,i,j]=plotobj(k)

[x,y]=meshgrid(0:0.01:1, 0:0.01:1);
xp = x.^k;
yp = y.^k;
z = 1+xp+yp;

f = (1+xp.*x+yp.*y)./z./(1-(1-1./z).*(1-x./z).*(1-y./z));
g = (1+xp.*x+yp.*y)./z;
figure(1);
surf(x,y,f);
shading flat

figure(2)
surf(x,y,g);
shading flat

[dummy,i]=max(f(:));

[i,j]=ind2sub(size(x),i);
r(1)=1;
r(2)=x(i,j);
r(3)=y(i,j);
