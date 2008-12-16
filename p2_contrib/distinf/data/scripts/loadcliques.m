function [c,t]=loadcliques(filename, n, dt)

s = loadsql(filename, 'select * from cliques');

c=javaArray('prl.domain', 1, n);
t=[];
k=1;

if ~isempty(s)
  [dummy,i] = sort([s.time]);
  s = s(i);
  start_time = s(1).time;
  cliques(1:n) = prl.domain;
  cliques(s(1).node) = s(1).clique;
  
  for i=2:length(s)
    if s(i).time > start_time+dt
      t(k,1) = s(i-1).time;
      c(k) = java.util.Arrays.copyOf(cliques, n);
      k = k + 1;
      start_time = s(i-1).time;
    end
    cliques(s(i).node) = s(i).clique;
  end

  t(k,1) = s(end).time;
  c(k) = java.util.Arrays.copyOf(cliques, n);

end

% warning: c(end, end) does not return correct results (seems to return
% c(end,1) rather than c(end, n)
