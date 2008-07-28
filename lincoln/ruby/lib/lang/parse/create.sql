create table Term (
	termid Integer,
	ruleid Integer,
	term_pos Integer,
	term_txt String
) keys (0);

create table Predicate (
	predicateid Integer,
	termid Integer,
	pred_pos Integer,
	pred_txt String

) 
keys(0);

create table PrimaryExpression (
	primaryexpressionid Integer,
	termid Integer,
	p_pos Integer,
	p_txt String,
	type String,
	datatype String
) keys (0);

create table Expression (
	expressionid Integer,
	termid Integer,
	expr_pos Integer,
	expr_text String
) keys(0);

create table Fact (
	factid Integer,
	programid Integer,
	tablename String
) keys(0);

create table Table (
	tableid Integer,
	tablename String
) keys(0);

create table Column (
	columnid Integer,
	tableid Integer,
	datatype String
) keys(0);

create table MyIndex (
	indexid Integer,
	tableid Integer,
	indx_pos Integer
) keys(0);
