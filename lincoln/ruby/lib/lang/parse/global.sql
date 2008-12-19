table index (
  +tablename TableName,
  +key Key,
  type TableType,
  classname String,
  object String
)

table operator (
  program String,
  rule String,
  +id String,
  selectivity Float
)

table function (
  +program String,
  +rule String,
  +position Integer,
  name String,
  object String
)