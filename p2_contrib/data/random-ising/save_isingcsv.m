function save_isingcsv(model, basename)

m = model.m;
n = model.n;

% Store the edges of the graphical model
[is,js] = find(model.edgelam);
f = fopen([basename '-edges.csv'], 'wt');
fprintf(f, '%d,%d\n', [is js]');
fclose(f);

% Store the node potentials
f = fopen([basename '-nodepot.csv'], 'wt');
np = model.nodepot(:)';
fprintf(f, '%d,%.16f_%.16f\n', [(1:m*n); np; 1-np]);

fclose(f);

% Store the edge potentials
[i,j,w] = find(model.edgelam);
f = fopen([basename '-edgepot.csv'], 'wt');
fprintf(f, '%d,%d,%.16f_%.16f_%.16f_%.16f\n', ...
        [i j exp(w) exp(-w) exp(-w) exp(w)]');
fclose(f);
