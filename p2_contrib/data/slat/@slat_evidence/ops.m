function values = ops(ev)
%function values = ops(ev)
% Number of observations per (time) step

values=zeros(size(ev));

for i=1:numel(ev)
  s = get(ev(i));
  values(i)=sum(sum(s.visible,1))/length(s.time);
end
