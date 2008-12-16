function values = opc(ev)
%function values = opc(ev)
% Number of observations per camera

values=zeros(size(ev));

for i=1:numel(ev)
  s = get(ev(i));
  values(i)=sum(sum(s.visible,1))/ncams(testbed(ev(i)));
end
