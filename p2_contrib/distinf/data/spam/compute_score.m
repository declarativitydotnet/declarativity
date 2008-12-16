function [score,cluster]=compute_score(ctr, a)
%a = a./repmat(sum(a,2),1,size(a,2));
score=a*ctr';
[score,cluster]=max(score,[],2);
