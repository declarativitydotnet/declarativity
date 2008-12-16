function centralizedRun(basename, day, hour, orig_cavg, final_cavg)
tic

time = strcat(day, '_', hour);

rName = strcat(basename, '_', time, '.dat');
rUsed = strcat(basename, '_', time, 'used.dat');
rMat = load(char(rName));
numIP = size(rMat, 1);
totalSamples = ceil(numIP / 10000);
iInit = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', '31', '32', '33', '34', '35', '36', '37', '38', '39', '40', '41', '42', '43', '44', '45', '46', '47', '48', '49', '50'};   
   
for k=1:totalSamples,
    sName = strcat(iInit(k), 'S', time, 'R.dat');
    
    if (k == totalSamples)
        numSamples = int2str(rem(numIP, 10000));
    else
        numSamples = '10000';
    end
    system(['./subSample.pl ', char(rName), ' ', char(sName), ' ', char(rUsed), ' ', char(numSamples)]);
    if (k == 1)
        system(['touch ', char(orig_cavg)]);
        system(['cat ', char(orig_cavg), ' >> ', char(sName)]);
    else 
        filename = strcat(iInit(k-1), 'c_avg');
        system(['cat ', char(filename), ' >> ', char(sName)]);
    end
    mat = load(char(sName));
    smat = sparse(mat);
    [s p] = createSim(smat);
    s = sparse(s);
    p = sparse(p);
    [idx, blah1, blah2, blah3] = apclustermex(s,p);
    unique_idx = unique(idx);
    exemplars = zeros(size(unique_idx,1),2);
    c_avg = zeros(size(exemplars,1), size(smat, 2));
    for i=1:size(exemplars,1)
        exemplars(i) = unique_idx(i);
        r = find(idx(:) == exemplars(i, 1));
        exemplars(i, 2) = size(r, 1);
        %sum the sending patterns of the IP addresses in the cluster and
        %divide by the number of IP addresses present
        for j=1:exemplars(i, 2)
            c_avg(i, :) = c_avg(i, :) + smat(r(j), :);
        end
        c_avg(i,:) = c_avg(i,:)./exemplars(i, 2);
    end
    filename = strcat(iInit(k), 'c_avg');
    if (k == totalSamples)
        save(char(final_cavg), 'c_avg', '-ASCII');
    else
        save(char(filename), 'c_avg', '-ASCII');
    end
    display(k);
end
for k=1:totalSamples-1,
    filename = strcat(iInit(k), 'c_avg');
    system(['rm ', char(filename)]);
end
toc