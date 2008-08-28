table index (
  +tablename TableName,
  +key Key,
  type TableType,
  classname String,
  object String
)

table query (
	  program String,
	  rule String,
	  public Integer,
	  delete Integer,
	  event String,
	  input TableName,
	  output TableName,
	  object String
)

table operator (
  program String,
  rule String,
  +id String,
  selectivity Float
)

table watch (
  +program String,
  +tablename String,
  +modifier String
)

