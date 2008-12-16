function s = loadbandwidth(name, s)
  
s.kb = sum_nodes(load([name '-tuplesize.txt']), s.t)/1024;
s.count = sum_nodes(load([name '-tuplecount.txt']), s.t);
if ~isempty(s.kb)
  s.bw = diff([s.kb(1,:); s.kb])/((s.t(2)-s.t(1))/s.epoch); % [kb/epoch]
  s.totalcomm = s.kb(:,1)';
  s.algs = {'total' 'experiment' 'rst' 'jt' 'inference' 'aggregation'};
end
