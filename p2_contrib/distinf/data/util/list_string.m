function str=list_string(separator, varargin)
% function str=list_string(separator, vector_or_text, ...);
% Creates a string from the provided numeric vectors.
% Examples:
% list_string(',',1:5) 
% -->  '1,2,3,4,5'
% list_string(';','(a=',1:3,',b=',4:6,')');
% -->  '(a=1,b=4);(a=2,b=5);(a=3,b=6)
% list_string(',',{'ab' 'bc' 'cd'})
% -->  'ab,bc,cd'

% Compute the storage requirement and check the data types
storage = length(separator);
nelems = 0;
for i=1:length(varargin)
  arg = varargin{i};
  if ischar(arg)
    storage = storage + length(arg);
  elseif (isvector(arg) || isempty(arg)) && isnumeric(arg)
    storage = storage + 20; % a rather loose bound
    nelems = max(nelems, length(arg));
  elseif (isvector(arg) || isempty(arg)) && iscell(arg)
    maxlen = 0;
    for j=1:length(arg)
      maxlen=max(maxlen,length(arg{j}));
      assert(ischar(arg{j}));
    end
    storage = maxlen;
    nelems = max(nelems, length(arg));
  else
    error('Only numeric or character data types supported for now.');
  end
end

if nelems>0
  str(1:nelems*storage)=char(0);
  next=1;

  for i=1:nelems,
    str(next:next+length(separator)-1)=separator;
    next = next + length(separator);
    for j=1:length(varargin),
      arg = varargin{j};
      if ischar(arg),
        object_str = arg;
      elseif iscell(arg)
        object_str = arg{i};
      else
        object_str = sprintf('%d',arg(i));
      end
      str(next:next+length(object_str)-1)=object_str;
      next=next+length(object_str);
    end
  end

  str = str(length(separator)+1:next-1);
  
else
  str='';
end
