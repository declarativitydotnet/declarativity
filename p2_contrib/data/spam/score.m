function [c_avg_orig, exemplars, scoreMat, newRowMap] = score(idx, rowMap, subSample, origMat, basename);
%rowMap = .dat file which stores the original IP address row number.
%subSample.
%idx vector that has exemplar for all the IP addresses
%basename - e.g. subsample-2007030106

unique_idx = unique(idx(:,2));
exemplars = zeros(size(unique_idx,1),2);
c_avg_orig = zeros(size(exemplars,1), size(subSample, 2));
cluster = zeros(size(exemplars,1), size(subSample, 2));
for i=1:size(exemplars,1)
    exemplars(i) = unique_idx(i);
    r = find(idx(:,2) == exemplars(i, 1));
    exemplars(i, 2) = size(r, 1);
    %sum the sending patterns of the IP addresses in the cluster and
    %divide by the number of IP addresses present
    for j=1:exemplars(i, 2)
        cluster(i, :) = cluster(i, :) + subSample(r(j), :);
    end
    c_avg_orig(i,:) = cluster(i,:)./exemplars(i, 2);
end
    
scoreMat = zeros(size(origMat,1),size(c_avg_orig, 1));
for i=1:size(c_avg_orig,1)
    norm = sum(c_avg_orig(i, :));
    scoreMat(:, i) = ((origMat*c_avg_orig(i, :)')./norm);
end

path = pwd;
filename = strcat(pwd, '/', basename,'-','score.dat');
save(filename, 'scoreMat', '-ASCII');

   
newRowMap = rowMap;
count = size(newRowMap,1);
for i=1:size(scoreMat,1)
    u = find(rowMap(:,2) == i);
    if (size(u,1) == 0)
        [val I] = max(scoreMat(i,:));
        if(val > 25)
            newRowMap(count+1, :) = [count+1 i rowMap(size(rowMap,1),3)];
            exemplars(I, 2) = exemplars(I, 2) + 1;
            cluster(I, :) = cluster(I, :) + origMat(i,:);
            count = count + 1;
        end
    end
end
    
c_avg_approx = zeros(size(exemplars,1), size(subSample, 2));
for i=1:size(exemplars,1)
    c_avg_approx(i,:) = cluster(i,:)./exemplars(i, 2);
end

path = pwd;
filename = strcat(pwd, '/', basename,'-','cavgApprox.dat');
save(filename, 'c_avg_approx', '-ASCII');

f = fopen([basename '-rowMap.dat'], 'wt');
for i=1:size(newRowMap,1)
    fprintf(f, ['%d %d %d\n'], newRowMap(i, :));
end
fclose(f);