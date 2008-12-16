function [s, p] = spconfigs(mat, nNodes, nVariables, basename)
 
[s, p] = createSim(mat);
[row, col, val] = find(s);
rns=randn('state'); randn('state',0); 
val=val+(rand(size(val,1),1)*1e-08);
p=p+(rand(size(p,2),1)*1e-08)';
s_sparse = s;
s = [row, col, val];
%load(matFile, 's', 'p')


if(mod(nVariables,nNodes) == 0)
    if size(s,2)==3
        f = fopen([basename '-' num2str(nVariables) '-preference-' num2str(nNodes) '.csv'], 'wt');
        for i=1:nVariables
            fprintf(f, ['%d,%.16g\n'], [i p(i)]);
        end
        fclose(f);

        f = fopen([basename '-' num2str(nVariables) '-similarity-' num2str(nNodes) '.csv'], 'wt');
        for i=1:size(s,1)
            fprintf(f, ['%d,%d,%.16g\n'], [s(i, :)]);
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
    else
        sprintf('%s', 'Similarity matrix not sparse')
    end
    filename = strcat(basename, '-', num2str(nVariables), '-', num2str(nNodes), '.mat');
    save(filename, 's','p');
else
    sprintf('%s', 'No of nodes should be a mutiple of the number of variables')
end


