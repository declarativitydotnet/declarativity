function apconfigs(matFile, nNodes, nVariables, basename)
%apconfigs('FaceClustering.mat', 5, 10, 'Face')

load(matFile, 's', 'p')

p_subset = zeros(nVariables, 1);
s_rows = (nVariables*nVariables)-nVariables
s_subset = zeros(s_rows, 3);

if(mod(nVariables,nNodes) == 0)
    f = fopen([basename '-' num2str(nVariables) '-preference-' num2str(nNodes) '.csv'], 'wt');
    for i=1:nVariables
        fprintf(f, ['%d,%d\n'], [i p(i)]);
        p_subset(i) = p(i);
    end
    fclose(f);

    f = fopen([basename '-' num2str(nVariables) '-similarity-' num2str(nNodes) '.csv'], 'wt');
    count = 1;
    for i=1:length(s)
        if(s(i,1) <= nVariables && s(i,2) <= nVariables)
            fprintf(f, ['%d,%d,%d\n'], [s(i, :)]);
            s_subset(count, :) = s(i, :);
            count = count + 1;
        end
    end
    fclose(f);
    
    f = fopen([basename '-' num2str(nVariables) '-vars-' num2str(nNodes) '.csv'], 'wt');
    vars = 1;
    for i=1:nNodes
        for j=1:nVariables/nNodes
            fprintf(f, ['%d,%d\n'], [i vars]);
            vars = vars + 1;
        end
    end
    fprintf(f, 'Done,Done\n');
    fclose(f);
    p = p_subset;
    s = s_subset;
    subsetMatFile = [basename '-' num2str(nVariables) '.mat']
    save(subsetMatFile, 'p', 's');
else
    sprintf('%s', 'No of nodes should be a mutiple of the number of variables')
end


