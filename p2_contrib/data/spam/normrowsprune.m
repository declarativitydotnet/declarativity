function b=normrowsprune(a)
b=normrows(a);
b=b(~isnan(b(:,1)),:);
