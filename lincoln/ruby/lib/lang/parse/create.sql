table Term (
	+termid Integer,
	ruleid Integer,
	term_pos Integer,
	term_txt String
);

table Predicate (
	+predicateid Integer,
	termid Integer,
	pred_pos Integer,
	pred_txt String
);

table PrimaryExpression (
	+primaryexpressionid Integer,
	termid Integer,
	p_pos Integer,
	p_txt String,
	type String,
	datatype String
);

table Expression (
	+expressionid Integer,
	termid Integer,
	expr_pos Integer,
	expr_text String
);

table Fact (
	+factid Integer,
	programid Integer,
	tablename String
);

table Table (
	+tableid Integer,
	tablename String
);

table Column (
	+columnid Integer,
	tableid Integer,
	datatype String
);

table MyIndex (
	+indexid Integer,
	tableid Integer,
	indx_pos Integer
);

table Program (
	+programid Integer,
	owner String
);