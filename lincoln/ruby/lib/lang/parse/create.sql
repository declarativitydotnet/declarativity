table MyTerm (
	+termid Integer,
	ruleid Integer,
	term_pos Integer,
	term_txt String
)

table MyPredicate (
	+predicateid Integer,
	termid Integer,
	pred_pos Integer,
	pred_txt String,
	event_mod String
)

table predicate (
  +program String,
  +rule String,
  +position Integer, 
  event String,
  object String
)

table MyPrimaryExpression (
	+primaryexpressionid Integer,
	expressionid Integer,
	p_pos Integer,
	p_txt String,
	type String,
	datatype String
)

table MyAssignment (
	+assignmentid Integer,
	termid Integer,
	assign_pos Integer,
	lhs String,
	assign_txt String
)

table MySelection (
	+selectionid Integer,
	termid Integer,
	select_pos Integer,
	select_txt String
)

table MyExpression (
	+expressionid Integer,
	termid Integer,
	arg_pos Integer,
	expr_pos Integer,
	expr_text String
)

table MyFact (
	+factid Integer,
	termid Integer,
	tablename String
)

table fact (
	program String, 
  tablename TableName,
	tuple Tuple
)

table MyTable (
  +tableid Integer,
  tablename String
)

table MyColumn (
	+columnid Integer,
	tableid Integer,
	col_pos Integer,
	datatype String
)

table MyIndex (
	+indexid Integer,
	tableid Integer,
	col_pos Integer
)


table index (
  +tablename TableName,
  +key Key,
  type TableType,
  classname String,
  object String
)

table MyProgram (
	+programid Integer,
	owner String,
	program_name String
)

table program (
	+program String,
	owner String,
	object String
)

table MyRule (
	+ruleid Integer,
	programid Integer,
	rulename String,
	public Integer,
	delete Integer
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

table compiler (
  +name String,
  owner String,
  file String,
  program String  
)

table assignment (
	+program String,
	+rule String,
  position Integer,
 object String
)

table rule (
  +program String,
  +name String,
  is_public String, 
  is_delete String,
  object String
)

table selection (
  +program String,
  +rule String,
  +position Integer,
  object String
)

table watch (
  +program String,
  +tablename String,
  +modifier String
)
