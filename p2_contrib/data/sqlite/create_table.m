function create_table(stmt, table, varargin)

columns=sprintf('%s, ', varargin{:});
stmt.executeUpdate(['create table ' table ' (' columns(1:end-2) ')']);
