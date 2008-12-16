function plotwgraph(a,loc,varargin)
%function plotwgraph(a,loc,varargin)

[scale,range] = process_options(varargin, 'scale', 'log', 'range', []);

N = size(a,1);

maxw = max(a(:));

switch scale
  case 'log' 
    if isempty(range)
      minw = min(a(:));
      range = maxw/minw;
    else
      minw = maxw/range;
    end
  case 'linear'
    if isempty(range)
      minw = 0; %min(a(:));
      range = maxw-minw;
    else
      minw = maxw-range;
    end
  otherwise
    error('invalid scale');
end

lrange = max(max(loc(1,:))-min(loc(1,:)),max(loc(2,:))-min(loc(2,:)));

washeld=ishold;

for i=1:N
  for j=i+1:N
    if a(i,j)>=minw
      delta = [loc(2,j)-loc(2,i);loc(1,i)-loc(1,j)];
      delta = delta/norm(delta)*lrange/300;
      h=fill([loc(1,[i j])+delta(1) loc(1,[j i])-delta(1)], ...
        [loc(2,[i j])+delta(2) loc(2,[j i])-delta(2)], 'b');
      switch scale
        case 'log'
          w = log(a(i,j)/minw)/log(range);
        case 'linear'
          w = (a(i,j)-minw)/range;
        otherwise
          error('invalid scale');
      end
      set(h,'facealpha',w,'linestyle','none');
      hold on;
    end
  end
end

if ~washeld
  hold off;
end