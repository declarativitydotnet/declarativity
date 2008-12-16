function [hb,ht] = textbar(labels, Y, varargin)
% function [hb,ht] = textbar(labels, Y, varargin)
hb = bar(Y, varargin{:});
set(gca,'xtick',[])

for i=1:size(Y,1)
  ht(i) = text(i,0,labels{i});
  set(ht(i),'verticalalignment','top','horizontalalignment','center');
end
