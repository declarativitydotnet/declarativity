function result=savesql(filename, table, s)
% result=savesql(connection, table, s)
% This function opens the SQLite database specified by filename,
% and stores the structure s in a table
%
%s Todo: mirror the save command more closely?

import java.sql.*;
import prl.*;

fields = fieldnames(s);

conn = sqlite.connect(absfile(filename));
stat = conn.createStatement;
drop_table(stat, table);
create_table(stat, table, fields{:});

values = repmat('?, ', 1, length(fields));
values = values(1:end-2);

prep = conn.prepareStatement(['insert into ' table ' values (' values ')']); 

c = struct2cell(s(:))';

for i = 1:size(c, 1)
  for j = 1:size(c, 2)
    value = c{i,j};
    if isa(value, 'prl.serializable_object')
      % eventually, we may integrate this with Java
      archive = xml_ostringarchive;
      value.save(archive);
      bytes = uint8(char(archive.str)); %java.io.ByteArrayInputStream();
      prep.setBytes(j, bytes);
    else
      prep.setObject(j, value);
    end
  end
  prep.addBatch;
end

conn.setAutoCommit(false);
prep.executeBatch;
conn.setAutoCommit(true);

%prep.close;
stat.close;
