function tmatScore = centralized(rmat, tmat);

    [s,p] = createSim(rmat);
    [idx, netsim, dpsim, expref] = apcluster(s, p);
    y = 1:size(idx);
    plot(idx,y, 'bo');
    hold on;
    
    
    exemplars = unique(idx)
    tmatScore = zeros(size(tmat,1),2);
    
    for i = 1:size(tmat, 1),
        tmatClassify = zeros(size(exemplars));
        for j = 1: size(exemplars,1),
            tmatClassify(j) = dot(tmat(i,:), rmat(exemplars(j),:));
        end;
        
        [M, index] = max(tmatClassify);
        tmatScore(i,:) = [M, exemplars(index)];
    end;
    
    hold off;
    figure;
    count = 1;
    for i = 1:size(tmat, 1),
        if tmatScore(i,1) > 0
            plot(tmatScore(i,2), tmatScore(i,1), 'ro');
            hold on;     
        end;
    end;
        
    