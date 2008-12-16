function [b,i]=prune_matrix(a, i)

if ~exist('i','var')
  % Take all domains with at least N / 1000 received messages
  % (N = # IP addresses)
  i = sum(a,1) > size(a,1)/1000;
end

b = a(:,i);

% Remove all IP addresses that communicated with <=1 domain
j = find(sum(b,2)>2);
b = b(j,:);
