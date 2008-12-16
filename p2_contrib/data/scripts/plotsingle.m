function plotsingle(s,type)
if nargin>1 && isnumeric(type)
  offset = type;
else
  offset = 0;
end

figure(1+offset);
plot(s.e, s.bw/s.n);
set(gca, 'fontsize', 16);
xlabel('iteration');
ylabel('per-node bandwidth [kB/iteration]');
legend(s.algs);
if nargin>1 && ischar(type)
  print(['-d' type],['plots/' s.name '-bandwidth']);
end

figure(2+offset);
plot(s.e, s.kb/s.n);
xlabel('iteration');
ylabel('total communication per node [kB]');
legend(s.algs);
if nargin>1 && ischar(type)
  print(['-d' type],['plots/' s.name '-size']);
end

n=3;
if isfield(s,'rms')
  figure(n+offset);
  plot(s.e, s.rms);
  xlabel('epoch');
  ylabel('RMS [C]');
  if nargin>1 && ischar(type)
    print(['-d' type],['plots/' s.name '-rms']);
  end
  n=n+1;
end

if isfield(s,'residual')
  figure(n+offset);
  plot(s.kb(:,1)/s.n, s.residual);
  xlabel('total communication per node [kB]');
  ylabel('residual');
  if nargin>1 && ischar(type)
    print(['-d' type],['plots/' s.name '-residual']);
  end
end