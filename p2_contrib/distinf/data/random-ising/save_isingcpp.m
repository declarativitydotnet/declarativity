function save_isingcpp(model, filename)
m = model.m;
n = model.n;

f = fopen([filename '.net'], 'wt');
fprintf(f, '@Variables\n');
fprintf(f, '%d 2\n',1:m*n);
fprintf(f, '@End\n\n');

fprintf(f, '@Potentials\n');
fprintf(f, '1 %d %.16f %.16f\n', [(1:m*n)-1; model.nodepot(:)'; 1-model.nodepot(:)']);

[i,j,w] = find(model.edgelam);
fprintf(f, '2 %d %d %.16f %.16f %.16f %.16f\n', [i-1 j-1 exp(w) exp(-w) exp(-w) exp(w)]');
fprintf(f, '@End\n');
