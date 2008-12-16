function [rejected,accepted]=loaddata(name,n)
% example use: a=loaddata('20070301_06',100);

if nargin < 1
  n=15175;
end

root=getenv('EXPROOT');

entries=load([root filesep 'distinf/spam/data/domains_' num2str(n) '/sample_' name '.dat']);
rejected=sparse(entries(:,1),entries(:,2),entries(:,3),max(entries(:,1)),n);

if nargout>1
  entries=load([root filesep 'distinf/spam/data/domains_' num2str(n) '/accepted_' name '.dat']);
  accepted=sparse(entries(:,1),entries(:,2),entries(:,3),max(entries(:,1)),n);
end
