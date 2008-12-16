root=getenv('EXPROOT');
assert(~isempty(root));
load ([root filesep 'distinf/spam/data/domains_100/sample_20070301_06.dat'])
rejected=sparse(sample_20070301_06(:,1), sample_20070301_06(:,2), sample_20070301_06(:,3), 114311, 100);
load ([root filesep 'distinf/spam/data/domains_100/sample_20070301_12.dat'])
rmat = sparse(sample_20070301_12(:,1), sample_20070301_12(:,2), sample_20070301_12(:,3), 104764, 100);
load ([root filesep 'distinf/spam/data/domains_100/accepted_20070301_12.dat'])
amat = sparse(accepted_20070301_12(:,1), accepted_20070301_12(:,2), accepted_20070301_12(:,3), 8618, 100); 
b=normrows(rejected);
%args={'onlinephase','off','options',statset('Display','iter')};
[cluster,ctr]=kmeans(full(b),10, 'Distance','cosine', 'EmptyAction','singleton');
score_graph(ctr, amat, rmat);