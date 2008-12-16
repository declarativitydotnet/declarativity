ii=find(sum(rejected,2));
rejected=rejected(ii,:);
[clustercos,ctrcos]=kmeans(b1, 10, 'Distance','cosine', 'EmptyAction','singleton','onlinephase','off', 'Options',statset('Display','iter'));
[clustereuc,ctreuc]=kmeans(b1, 10, 'Distance','sqEuclidean', 'EmptyAction','singleton','onlinephase','off','Options',statset('Display','iter'));
