function plotllmog1(s,cluster,varargin)
for i=1:length(varargin)
  X = normrowsprune(varargin{i});
  p = pmog(X,s);
  [stuff, ind] = max(p,[],2);
  ii = find(ind==cluster);
  n = length(ii);
  args{1,i} = sort(log(sum(p(ii,:),2)));
  args{2,i} = (1:n)/n;
end
plot(args{:});
