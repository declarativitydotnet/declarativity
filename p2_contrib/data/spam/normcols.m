function [b,s]=normcols(a)
s=std(a,1,1);
s(s==0)=1;
b=a./repmat(s, size(a,1), 1);
