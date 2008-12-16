function reimport_distributed

curdir = pwd;

cd('~/slat');
!ls */*/saved > done.txt

fid = fopen('done.txt', 'rt');
line = fgetl(fid);
while ischar(line),
  dir = fileparts(line);
  cd(dir);
  a=load('id');
  if exist('./soln.xml', 'file')
    nsolns = mym('select count(*) from solution where experiment="{S}"', ...
      a.experiment_id);
    nmatch = mym('select count(*) from solution where id="{S}"', ...
      a.solution_id);
    nvalid = mym(['select count(*) from solution ' ...
      ' where experiment="{S}" and id = "{S}"'], a.experiment_id, a.solution_id);
    assert(nmatch == nvalid);
    display([a.experiment_id ': ' num2str(nsolns) ' solutions, '...
             num2str(nmatch) ' matching']);
    if nmatch==0 && nsolns>0
      soln_object = process1('.');
      save(soln_object);
    end
    line = fgetl(fid);
  end
  cd('~/slat');
  line = fgetl(fid);
end

cd(curdir);
