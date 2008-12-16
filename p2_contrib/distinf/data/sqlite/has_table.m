function b=has_table(filename, table)
    
conn = sqlite.connect(absfile(filename));
stat = conn.createStatement;
q = ['select count(*) from sqlite_master where name="' table '"'];
rs = stat.executeQuery(q);

b = logical(rs.getInt(1));

rs.close;
stat.close;
