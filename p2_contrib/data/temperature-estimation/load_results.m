function [results, time] = load_results(filename)
  
a = load(filename);

assert(size(a,2)==3);
n=a(end,1);

[dummy,i]=sort(a(:,end));
a = a(i,:);

done = zeros(1,n);

% preprocess
i=1;
while i<=size(a,1) && ~all(done)
  temp(a(i,1))=a(i,2);
  done(a(i,1))=1;
  i=i+1;
end

records = i-1:size(a,1);
results = zeros(length(records), n);
time = zeros(length(records),1);

for j=1:length(records),
  i = records(j);
  temp(a(i,1))=a(i,2);
  results(j,:) = temp;
  time(j) = a(i,end);
end

time=1+(time-time(1))*86400/2;
