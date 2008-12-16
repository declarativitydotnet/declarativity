function plot_nips(use_cache, type)

if use_cache
  load plot_nips
else
  %s=loadsoln('jtinf/emulab-54','intel/');
  s=loadsoln('jtsetinf/emulab-54','model');
  for i=1:length(s)
    msize(i,:) = s(i).kb(end,3:5);
    mcount(i,:) = s(i).count(end,3:5);
  end
  msize = mean(msize, 1);
  mcount = mean(mcount, 1);
  
  lisp=load('lisp_conv.mat');  % spanning tree, junction tree, inference
  msize(2,:) = mean(lisp.s/1024, 1);
  mcount(2,:) = mean(lisp.c, 1);
  %save plot_nips msize mcount
end

components={'routing' 'triangulation' 'inference'};
implementations={'this paper' '[IPSN05]'};

figure(1);
[hb,ht]=textbar(implementations, msize/54/100);
set([gca ht], 'fontsize', 20, 'fontname', 'times');
legend(components{:});
ylabel('bandwidth per node [kB/epoch]');
if nargin>1
  print(['-d' type], 'plots/ipsn05-comparison-size');
end

figure(2);
[hb,ht]=textbar(implementations, mcount/54/100);
set([gca ht], 'fontsize', 20, 'fontname', 'times');
legend(components{:});
ylabel('# messages per node [count/epoch] ');
if nargin>1
  print(['-d' type], 'plots/ipsn05-comparison-count'); 
end
