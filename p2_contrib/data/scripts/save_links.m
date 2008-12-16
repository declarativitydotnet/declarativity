function save_links(a, basename, nnodes)
% save_links(a, basename, nnodes)
% a is the connectivity matrix: a(i,j) = probability of receiving message
% from j at i
%
% Outputs links in the following format: node1, node2, psend, precv

an = a(1:nnodes,1:nnodes);
[i, j, precv] = find(an);
psend = full(an(sub2ind(size(an), j, i)));

f = fopen([basename '-links-' num2str(nnodes) '.csv'], 'wt');
fprintf(f, ['%d,%d,%g,%g\n'], [i j psend precv]');
fclose(f);
