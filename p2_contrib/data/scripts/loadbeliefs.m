function b=loadbeliefs(filename, n, t)

% % compute the beliefs
% beliefs = load([name '-beliefs.txt']);
% 
% nvars = max(beliefs(:,1));
% b = zeros(length(t), nvars)*nan;
% for i=1:length(t)
%   j = find(beliefs(:,end)<=t(i));
%   b(i,beliefs(j,1)) = beliefs(j,3);
% end
%   
% missing = find(all(isnan(b),1));
% if ~isempty(missing)
%   warning(['Missing beliefs for variables ' num2str(missing)]);
% end
% 
% disp(sprintf('%s: loaded %d records with %d variables', ... 
%              name, size(b)));

% compute the beliefs

s = loadsql(filename, 'select * from beliefs');

b=javaArray('prl.factor', 1, n);

[dummy,i] = sort([s.time]);
s = s(i);

node = [s.node];
time = [s.time];

for i=1:length(t)
  for j=1:n
    k = find(node == j & time <= t(i), 1, 'last');
    if ~isempty(k)
      b(i,j) = s(k).belief;
    else
      b(i,j) = prl.constant_factor(1);
    end
  end
end

