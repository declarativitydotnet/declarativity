function plotres(xaxis, varargin)
options = {};
for i=1:length(varargin)
  if ischar(varargin{i})
    options = varargin(i:end);
    break;
  end
  s = varargin{i};
  %checksame(s.(xaxis));
  args{1,i} = mean(vertcat(s.(xaxis)),1);
  args{2,i} = vertcat(s.residual)';
end

plotmean(args{:}, options{:});

xlabel(xaxis);
ylabel('residual');


% k=1; args={};

% for i=1:length(varargin)
%   s = varargin{i};
%   if isfield(s,'expresidual')
%     args{1,k} = mean(s.(xaxis),1);
%     args{2,k} = mean(s.expresidual,1);
%     args{3,k} = '--';
%     k=k+1;
%   end
% end

% if k>1
%   hold on;
%   plot(args{:});
%   hold off
% end
