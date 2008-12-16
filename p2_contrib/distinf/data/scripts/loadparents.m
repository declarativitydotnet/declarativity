function [parents,t]=loadparents(filename, dt)
a=load(filename);

if isempty(a)
  warning('No parents found');
  parents = [];
  t = [];
  return;
end
assert(size(a,2)==3);

a = sortrows(a, 3);
n = max(max(a(:,1:2)));

parents=[];
t=[];
pa = zeros(1,n);

start_time = a(1,end);
for i=1:size(a,1)
  pa(a(i,1))=a(i,2);

  if i==size(a,1) || a(i+1,end)>start_time+dt
    parents(end+1,:) = pa;
    t(end+1,1) = a(i,end);
    if i<size(a,1) 
      start_time = a(i+1,end);
    end
  end
end
%st = a(:,3);
