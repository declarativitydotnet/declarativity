function f=fields(object, include_parents)
if nargin==1 || include_parents
  f = fields(object.experiment);
else
  f = field;
end

f = add(f, field('evidence', sql_object_type('slat_evidence'),[],1));
f = add(f, field('model', 'slat_model', [], 1));
