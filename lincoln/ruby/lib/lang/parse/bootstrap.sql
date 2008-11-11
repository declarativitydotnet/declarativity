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

table compiler (
  +name String,
  owner String,
  file String,
  program String  
)