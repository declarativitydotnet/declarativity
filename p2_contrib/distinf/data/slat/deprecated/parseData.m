function [stats,stats1]=parseData(filename)
if ~exist('filename','var')
  filename='data.txt';
end

data=load(filename);
stats.step=data(:,1)'+1;
stats.time=data(:,2)';
stats.spanning=data(:,3)';
stats.rip=data(:,4)';
stats.nLiveNodes=data(:,5)';
stats.nPairs=data(:,6)';
stats.nInvalidBeliefs=data(:,7)';
stats.nLocallyValid=data(:,8)';
stats.nLocallyAligned=data(:,9)';
stats.nPairwiseAligned=data(:,10)';
stats.sumPairwiseKL=data(:,11)';
stats.distRMS=data(:,12)';

% stopper
data=[data;ones(1,size(data,2))*(data(end,1)+1)];
indices=[];

for i=2:length(stats.step),
  if stats.step(i)~=stats.step(i-1)
    indices=[indices (i-1)];
  end
end
    
stats1.step=data(indices,1)'+1;
stats1.time=data(indices,2)';
stats1.spanning=data(indices,3)';
stats1.rip=data(indices,4)';
stats1.nLiveNodes=data(indices,5)';
stats1.nPairs=data(indices,6)';
stats1.nInvalidBeliefs=data(indices,7)';
stats1.nLocallyValid=data(indices,8)';
stats1.nLocallyAligned=data(indices,9)';
stats1.nPairwiseAligned=data(indices,10)';
stats1.sumPairwiseKL=data(indices,11)';
stats1.distRMS=data(indices,12)';
