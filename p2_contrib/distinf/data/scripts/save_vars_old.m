function save_vars(prefix, vars, vartype, basename, mode, close)

if ~exist('mode','var')
  mode = 'w';
end

if ~exist('close','var')
  close = 1;
end

nnodes = length(vars);

f = fopen([basename '-vars-' num2str(nnodes) '.csv'], [ mode 't']);
for i=1:length(vars)
  varsi = reshape(vars{i},1,[]);
  fprintf(f, ['%d,' prefix '%d,' vartype '\n'], ...
          [repmat(i,1,length(varsi)); varsi]);
end

if close
  fprintf(f, 'Done,0,%s\n', vartype);
end

fclose(f);
