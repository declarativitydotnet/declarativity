function drop_table(stmt, name)
stmt.executeUpdate(['drop table if exists ' name]);
