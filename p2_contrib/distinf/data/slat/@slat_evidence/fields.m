function f=fields(object, include_parents)
f = field;
f = add(f, field('data', sql_object_type('slat_data')));
f = add(f, field('algorithm','observation_extractor'));
f = add(f, field('visible'));
f = add(f, field('obs'));
f = add(f, field('time'));

