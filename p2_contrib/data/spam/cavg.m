function [c_avg_orig, exemplars] = cavg(idx, subSample, basename);
%rowMap = .dat file which stores the original IP address row number.
%subSample.
%idx vector that has exemplar for all the IP addresses
%basename - e.g. subsample-2007030106

    unique_idx = unique(idx(:,2))
    exemplars = zeros(size(unique_idx,1),2)
    c_avg_orig = zeros(size(exemplars,1), size(subSample, 2));
    cluster = zeros(size(exemplars,1), size(subSample, 2));
    size(cluster)
    for i=1:size(exemplars,1)
        exemplars(i) = unique_idx(i);
        r = find(idx(:,2) == exemplars(i, 1))
        exemplars(i, 2) = size(r, 1)
        %sum the sending patterns of the IP addresses in the cluster and
        %divide by the number of IP addresses present
        for j=1:exemplars(i, 2)
            cluster(i, :) = cluster(i, :) + subSample(r(j), :);
        end
        c_avg_orig(i,:) = cluster(i,:)./exemplars(i, 2);
    end
    
    path = pwd;
    filename = strcat(pwd, '/', basename,'-','cavg', '.dat')
    save(filename, 'c_avg_orig', '-ASCII');
