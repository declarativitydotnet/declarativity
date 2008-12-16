function values = sopc(ev)
%function values = sopc(ev)
% Total number of shared observations per camera

values=zeros(size(ev));

for i=1:numel(ev)
  s = get(ev(i));
  shared = max(sum(s.visible,1)-1,0);
  values(i)=sum(shared)/ncams(testbed(ev(i)));
end
