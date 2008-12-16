function save_imgcsv(img, basename)

m = img.rows;
n = img.cols;

[is,js] = find(grid_adjacency(m,n));

% Store the edges of the graphical model
f = fopen([basename '-edges.csv'], 'wt');
fprintf(f, '%d,%d\n', [is js]');
fclose(f);

% Store the node potentials
f = fopen([basename '-nodepot.csv'], 'wt');
p0 = exp(-(img.data(:)-img.mu(1)).^2/img.sigma(1))';
p1 = exp(-(img.data(:)-img.mu(2)).^2/img.sigma(2))';
fprintf(f, '%d,%.16g_%.16g\n', [(1:m*n); p0; p1]);
fclose(f);

% Store the edge potentials
f = fopen([basename '-edgepot.csv'], 'wt');
w = repmat(exp(-2),length(is),1);
fprintf(f, '%d,%d,1_%.16g_%.16g_1\n', [is js w w]');
fclose(f);
