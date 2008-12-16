function plotllmog(s,varargin)
for i=1:length(varargin)
  X = normrowsprune(varargin{i});
  n = size(X,1);
  args{1,i} = (1:n)/n;
  args{2,i} = sort(llmog(X,s));
end
plot(args{:});
