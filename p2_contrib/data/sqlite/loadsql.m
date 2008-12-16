function result=loadsql(filename, query)
% result=loadsql(filename, query)

global u;

import java.sql.*;
import prl.*;

conn = sqlite.connect(absfile(filename));
stat = conn.createStatement;
rs = stat.executeQuery(query);
rsmd = rs.getMetaData;

n = rsmd.getColumnCount;

% Get information about the field types
for i=1:n
  fields{i} = char(rsmd.getColumnLabel(i));
end

values = {};
i = 1;

while rs.next
  for j=1:n
    switch rsmd.getColumnType(j)
      case Types.NULL
        values{i,j} = [];
      case Types.INTEGER 
        values{i,j} = rs.getInt(j); % Should this be getLong?
      case Types.FLOAT;
        values{i,j} = rs.getDouble(j);
      case Types.VARCHAR;
        values{i,j} = char(rs.getString(j));
%         bytes = rs.getBytes(j);
%         xml = xml_iarchive(bytes, u);
%         values{i,j} = xml.read;
      case Types.BLOB;
        bytes = rs.getBytes(j);
        xml = xml_iarchive(bytes, u);
        values{i,j} = xml.read;
      otherwise
        type = rsmd.getColumnType(j);
        rs.close;
        stat.close;
        error(['Unknown type ' num2str(type)]);
    end
  end
  i = i + 1;
end

result = cell2struct(values, fields, 2);
rs.close;
stat.close;

