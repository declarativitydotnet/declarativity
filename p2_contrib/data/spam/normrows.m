function b=normrows(a)
b=a./repmat(sum(a,2),1,size(a,2));
