function f = fields(obj, include_parents)
if nargin==1 || include_parents
  f = fields(obj.solution);
else
  f = field;
end

f = add(f, field('xt'));
f = add(f, field('Pt'));
f = add(f, field('xtl'));
f = add(f, field('Ptl'));
f = add(f, field('tree'));
f = add(f, field('ttree'));
f = add(f, field('stats'));

  
