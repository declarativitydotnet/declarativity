root=getenv('EXPROOT');
assert(~isempty(root));
rejected = load ('plots/ap/rejected/random-run/1S01_24R.dat');
load ([root filesep 'distinf/spam/data/domains_20/sample_20070302_24.dat'])
rmat = sparse(sample_20070302_24(:,1), sample_20070302_24(:,2), sample_20070302_24(:,3), 237314, 20);
load ([root filesep 'distinf/spam/data/domains_20/accepted_20070302_24.dat'])
amat = sparse(accepted_20070302_24(:,1), accepted_20070302_24(:,2), accepted_20070302_24(:,3), 46154, 20); 
[b s]=normcols(rejected);
[cluster,ctr]=kmeans(b,19, 'Distance','sqEuclidean', 'EmptyAction','singleton');
ctr_orig = ctr .* repmat(s, size(ctr,1), 1);
score_graph(ctr_orig, amat, rmat, 'sample_20070301_24_sqEuclidean_normcol');
