function [decomposable,flat,vars]=load_savedmodel(filename)

s = loadsql(filename, 'select * from decomposable');
decomposable=[s.factor];

s = loadsql(filename, 'select * from flat');
flat = [s.factor];

s = loadsql(filename, 'select * from variables');
vars = [s.variables];
