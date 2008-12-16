function plotAlignmentStats(varargin)
colors='rmbgc';

args=[];
for i=1:length(varargin)
  args=[args varargin{i}];
end

figure(10)
clf
hold on
for i=1:length(args)
  res=args(i).data;
  plot(res.nPairwiseAligned, colors(i));
end
ylabel('Number of pairwise aligned nodes');
legend(args.name);
hold off

figure(11)
clf
hold on
for i=1:length(args)
  res=args(i).data;
  plot(res.nPairwiseAligned./res.nPairs, colors(i));
end
ylabel('Proportion of pairwise aligned nodes');
legend(args.name);
hold off

figure(12)
clf
hold on
for i=1:length(args)
  res=args(i).data;
  semilogy(res.sumPairwiseKL, colors(i));
end
ylabel('Symmetrized KL divergence');
legend(args.name);
hold off
