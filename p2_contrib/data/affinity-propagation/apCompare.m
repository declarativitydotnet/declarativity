%pass this function the similarity, preferences used from the distributed
%the availability and responsibility should be the All version
%the exemplars should be the final version. 
%case, which will be used in the centralized version
%its is the number of iterations to use. 
function [clust1, clust2] = apCompare(s,p,its, aFile, rFile, eFile);
    
    % run apcluster.m with the appropriate data set and grab the
    % Availabilities, Responsibilities, and Exemplars (converged values).
    [eCentral,netsim,dpsim,expref,aCentral,rCentral]=apcluster(s,p,'nonoise', 'convits', its, 'maxits', its);

    %load in the values from the distributed version
    aFiled = dlmread(aFile);
    rFiled = dlmread(rFile);
    eFiled = dlmread(eFile);

    % grab the number of variables through size number. 
    [numVars] = size(p,2);

    % 5th row contains the iteration number
    aFiled = sortrows(aFiled, 4);
    rFiled = sortrows(rFiled, 4);

    %for loop to grab each avail,respons, exemplar values from each iteration
    aYVal = zeros(1, its);
    rYVal = zeros(1, its);
    aIt = zeros(numVars);
    rIt = zeros(numVars);
    curRowA = 1;
    curRowR = 1;
    for i = 0:its
      % at it. 
      [aIt, curRowA] = selectIteration(aFiled, i, curRowA, aIt);
      [rIt, curRowR] = selectIteration(rFiled, i, curRowR, rIt);
      %diff the availabilities
      aYVal(i+1) = diffMat(aIt, aCentral);
      rYVal(i+1) = diffMat(rIt, rCentral);
    end
    %plot values to see difference at each iteration
    x = 0:its;
    figure(1)
    plot(x, aYVal);
    xlabel('# Iterations');
    ylabel('Difference in all Availabilities');
    figure(2)
    plot(x, rYVal);
    xlabel('# Iterations');
    ylabel('Difference in all Responsibilities');
    %add test here to check values are close to each other
    %temporarily use 1 as the maxErrorBound
    maxErrorBound = 10.0;
    maxErrors = numVars * numVars * .1;
    %last av and re should have the last iteration saved
    %availabilities have large errors w/ sparse matrices because
    %they use positive and neg Inf (or close to that for responsibility
    %values, which throws availabilities off). 
    numErrors = checkSame(aIt, aCentral, maxErrorBound);
    if (numErrors > maxErrors)
        disp('Too many availability errors');
        %display(aIt);
        %display(aCentral);
    end
    numErrors = checkSame(rIt, rCentral, maxErrorBound);
    if (numErrors > maxErrors)
        disp('Too many responsibility errors');
        display(numErrors);
        display(rIt);
        display(rCentral);
    end

    %maxExemplarDiff = numVars * .1;
    eDist = zeros(numVars,1);
    for j = 1:numVars
        i = eFiled(j,1);
        eDist(i) = eFiled(j,2);
    end
    [clust1, clust2] = diffClusters(eDist, eCentral, 0.1);
end 

function [subMat,curRow] = selectIteration(mat, itNum, rowStart, prevMat);
    curRow = rowStart;
    subMat = prevMat;
    maxRows = size(mat,1);
    while (curRow <= maxRows && mat(curRow, 4) == itNum)
        i = mat(curRow, 1);
        k = mat(curRow, 2);
        subMat(i,k) = mat(curRow, 3);
        curRow = curRow + 1;
    end
end

function retVal = diffMat(mat1, mat2)
    [rS,cS] = size(mat1);
    retVal = 0;
    for i = 1:rS
        for k = 1:cS
            val1 = mat1(i,k);
            val2 = mat2(i,k);
            % they use really large neg and positive values, but its not
            % -Inf or +Inf, so just used a raw really large value.
            if (val2 <= -1.79e+308 || val2 >= 1.79e+308)
                val2 = 0;
            end
            retVal = retVal + abs(val1 - val2);
        end
    end
end

function [clust1, clust2] = diffClusters(eMat1, eMat2, maxPercentDiff)
    [rs,cs] = size(eMat1);
    ueMat1 = unique(eMat1);
    ueMat2 = unique(eMat2);
    [numUnique1, cs] = size(ueMat1);
    [numUnique2, cs] = size(ueMat2);
    clust1 = zeros(numUnique1, rs);
    clust2 = zeros(numUnique2, rs);
    if (numUnique1 ~= numUnique2)
        disp('total number of clusters are different');
        display(numUnique1);
        display(numUnique2);
        return;
    end
    for i = 1:numUnique1
        for j = 1:rs
            if (eMat1(j) == ueMat1(i))
                clust1(i,j) = eMat1(j);
            end
        end
    end
    for i = 1:numUnique2
        for j = 1:rs
            if (eMat2(j) == ueMat2(i))
                clust2(i,j) = eMat2(j);
            end
        end
    end

    %optimal matching START
    pairs1 = zeros(numUnique1);
    for i = 1:numUnique1,
        for j = 1:numUnique2,
            totalSame = 0;
            totalDiff = 0;
            for k = 1:rs
                if (clust1(i,k) ~= 0 && clust2(j,k) ~= 0)
                    totalSame = totalSame + 1;
                elseif (clust1(i,k) == 0 && clust2(j,k) ~= 0)
                    totalDiff = totalDiff + 1;
                elseif (clust1(i,k) ~= 0 && clust2(j,k) == 0)
                    totalDiff = totalDiff + 1;
                end
            end
            pairs1(i,j) = totalSame / (totalSame + totalDiff);
        end
    end
    pairs2 = pairs1';
    
    allMatched = 0;
    numUnique = numUnique1;
    while(~allMatched)
        %each cluster in clust1 proposes to best in clust2 that isn't
        %crossed off. 
        propose = zeros(numUnique);
        acceptList = zeros(numUnique);
        for i = 1:numUnique,
            [dc, maxp] = max(pairs1(i,:));
            propose(i, maxp) = 1;
            acceptList(maxp, i) = 1;
        end
        accepted = zeros(numUnique, 1);
        rejectMatrix = zeros(numUnique);
        for i = 1:numUnique,
            for j = 1:numUnique,
                if (acceptList(i,j) ~= 0)
                    if (accepted(i) == 0)
                       accepted(i) = j;
                    elseif (pairs2(i,j) > pairs2(i,accepted(i)))
                        accepted(i) = j;
                    else
                        rejectMatrix(j,i) = 1;
                    end
                end
            end
        end
        %proposers cross off anyone who has rejected them. 
        for i = 1:numUnique,
            for j = 1:numUnique,
                if (rejectMatrix(i,j) == 1)
                    %make it negative so it isn't picked again
                    pairs1(i,j) = -1;
                end
            end
        end
        %test if all have matched yet. 
        if (rejectMatrix == zeros(numUnique))
            allMatched = 1;
        end
    end
    for i = 1:numUnique
        if (pairs2(i, accepted(i)) < 1 - maxPercentDiff)
            display('Differing clusters');
            disp(clust1);
            disp(clust2);
        end
    end
end

function ErrorCount = checkSame(mat1, mat2, maxError)
    [rS, cS] = size(mat1);
    ErrorCount = 0;
    for i = 1:rS
        for k = 1:cS
            val1 = mat1(i,k);
            val2 = mat2(i,k);
            % they use really large neg and positive values, but its not
            % -Inf or +Inf, so just used a raw really large value.
            if (val2 <= -1.79e+308 || val2 >= 1.79e+308)
                val2 = 0;
            end
            if (abs(val1 - val2) > maxError)
                ErrorCount = ErrorCount + 1;
            end
        end
    end
end