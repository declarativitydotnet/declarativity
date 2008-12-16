function [mu,sigma]=discrete2gaussian(xd, x1, x2, x3)
x1=x1';
x2=x2';
x3=x3';

%if length(varargin)~=length(size(xd))
%  error('invalid number of parameters');
%end

xd=xd/sum(xd(:));

mu=ones(3,1);

mu(1)=sum(x1.*squeeze(sum(sum(xd,2),3)));
mu(2)=sum(x2'.*squeeze(sum(sum(xd,1),3)));
mu(3)=sum(x3.*squeeze(sum(sum(xd,1),2)));

sigma=zeros(3);
sigma(1,2)=sum(sum((x1*x2').*sum(xd,3)));
sigma(1,3)=sum(sum((x1*x3').*squeeze(sum(xd,2))));
sigma(2,3)=sum(sum((x2*x3').*squeeze(sum(xd,1))));

sigma=sigma+sigma';

sigma(1,1)=sum(x1.^2.*squeeze(sum(sum(xd,2),3)));
sigma(2,2)=sum(x2.^2.*squeeze(sum(sum(xd,1),3))');
sigma(3,3)=sum(x3.^2.*squeeze(sum(sum(xd,1),2)));

sigma=sigma-mu*mu';
sigma=(sigma+sigma')/2;

