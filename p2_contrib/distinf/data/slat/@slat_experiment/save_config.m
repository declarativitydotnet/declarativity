function save_config(ex, fid)

save_config(ex.experiment,fid);

fprintf(fid, '(load "filter/slat-model.lisp")\n');
