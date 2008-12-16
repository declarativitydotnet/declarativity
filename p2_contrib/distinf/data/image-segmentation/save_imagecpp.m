function save_isingcpp(img, filename)
m = img.rows;
n = img.cols;

f = fopen([filename '.net'], 'wt');
fprintf(f, '@Variables\n');
fprintf(f, '%d 2\n',1:m*n);
fprintf(f, '@End\n\n');

fprintf(f, '@Potentials\n');
p0 = exp(-(img.data(:)-img.mu(1)).^2/img.sigma(1))';
p1 = exp(-(img.data(:)-img.mu(2)).^2/img.sigma(2))';
fprintf(f, '1 %d %.16g %.16g\n', [(1:m*n)-1; p0; p1]);

a = grid_adjacency(m,n);
[i,j] = find(a);
w = exp(-2);
fprintf(f, '2 %d %d 1 %.16g %.16g 1\n', [i-1 j-1 repmat(w, size(i,1), 2)]');
fprintf(f, '@End\n');
