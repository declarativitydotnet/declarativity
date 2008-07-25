create table Term (
	termid Integer,
	ruleid Integer,
	position Integer,
	text String

) keys (0);

create table Predicate (
	predicateid Integer,
	termid Integer,
	position Integer,
	text String

) 
keys(0);

create table PrimaryExpression (
	primaryexpressionid Integer,
	termid Integer,
	position Integer,
	text String,
	type String,
	datatype String
) keys (0);

create table Expression (
	expressionid Integer,
	termid Integer,
	tablename String,
	position Integer,
	event String
) keys(0);

create table Fact (
	factid Integer,
	programid Integer,
	tablename String
) keys(0);
