function plot_tree(loc,parents,varargin)

if length(varargin)>0 && isnumeric(varargin{1})
  wt = varargin{1};
  varargin = varargin(2:end);
else
  wt = [];
end

[fontsize,highlight,groups] = ...
    process_options(varargin,'fontsize',8,'highlight',[],'groups',[]);


n = length(parents);  
washeld = ishold;
source = zeros(2,0);
delta  = zeros(2,0);

for i=1:n
  j = parents(i);
  if j
    source = [source loc(1:2,i)];
    delta  = [delta loc(1:2,j)-loc(1:2,i)];
  end 
end

h = quiver(source(1,:),source(2,:),delta(1,:),delta(2,:));
hold on;
zerop = find(parents==0);
plot(loc(1,zerop),loc(2,zerop),'ro');
if ~washeld
  hold off;
end
%   nextplot_f = get(gcf,'nextplot');
%   nextplot_a = get(gca,'nextplot');
%   set(gcf,'nextplot',nextplot_f);
%   set(gca,'nextplot',nextplot_a);

set(h,'autoscale','off');

if fontsize
  for j=1:n
    text(loc(1,j),loc(2,j),num2str(j),'fontsize',fontsize,'verticalalign','bottom');
  end
end


if ~isempty(wt)
  for i=1:n
    j = parents(i);
    if j
      textloc=num2cell(mean(loc(1:2,[i j]),2));
      h=text(textloc{:},num2str(round(wt(i))));
      set(h,'fontangle','italic');
      set(h,'color','b');
    end
  end
end


