function plotconv(xaxis, threshold, varargin)
options = {};
for i=1:length(varargin)
  if ischar(varargin{i})
    options = varargin(i:end);
    break;
  end
  s = varargin{i};
  %checksame(s.(xaxis));
  args{1,i} = mean(vertcat(s.(xaxis)),1);
  args{2,i} = mean(converged(vertcat(s.residual), threshold),1)*100;
end

%plotmean(args{:}, options{:});
plot(args{:});
xlabel(xaxis);
ylabel('% of converged runs');
ylim([0 100]);

% 
% function plotconv(xaxis, threshold, varargin)
% for i=1:length(varargin)
%   s = varargin{i};
%   if isfield(s, 'it')
%     assert(all(all(s.it==repmat(s.it(1,:),size(s.it,1),1))));
%   else
% %    assert(all(all(s.time==repmat(s.time(1,:),size(s.time,1),1))));
%   end
%   args{1,i} = mean(s.(xaxis),1);
%   args{2,i} = mean(converged(s.residual, threshold),1)*100;
% end
% 
