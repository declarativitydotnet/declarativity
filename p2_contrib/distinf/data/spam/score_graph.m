function score_graph(c_avg, amat, rmat, name);

figure(1)
plotCAscore(c_avg, amat, rmat);
fname = strcat('plots2/', name, '_1')
print('-depsc',char(fname));

val2 = c_avg ./ repmat(sum(c_avg, 2), 1, size(c_avg, 2));
[a b] = find(val2 >= 0.8);
a = sort(a);
a2 = a;
c_avg3 = c_avg;
a2 = sort(a); 
for i = 1:length(a2),
    c_avg3(a2(i), : )  = [];
    a2 = a2 -1;
end
figure(2)
plotCAscore(c_avg3, amat, rmat);
fname = strcat('plots2/', name, '_2')
print('-depsc',char(fname));