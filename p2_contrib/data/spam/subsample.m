function [b,i]=subsample(a,n);
i = randperm(size(a,1));
i=i(1:n);
b=a(i,:);
