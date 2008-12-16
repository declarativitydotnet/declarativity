function [hb, ht]=plotbwbars(field, threshold, cols, varargin)

varargin = reshape(varargin, 2, []);
for j=1:size(varargin,2)
  s = varargin{1,j};
  val =[];
  for i=1:length(s)
    t=find(s(i).(field) <= threshold, 1);
    if isempty(t)
      warning(sprintf('Threshold not reached at input %d, index %d',j,i));
    else
      val=[val; s(i).kb(t,cols)];
    end
  end
  y(j,:) = mean(val,1);
end
y = [y sum(y,2)];

[hb,ht]=textbar(varargin(2,:), y);
