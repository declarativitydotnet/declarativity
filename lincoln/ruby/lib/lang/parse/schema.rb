require 'lib/types/table/object_table'
class ColumnTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		COLUMNID=0
		TABLEID=1
		DATATYPE=2

	end
	@@SCHEMA = [Integer,Integer,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "ColumnTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::COLUMNID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		columnid = Variable.new("columnid",Integer)
		columnid.position=0
		tableid = Variable.new("tableid",Integer)
		tableid.position=1
		datatype = Variable.new("datatype",String)
		datatype.position=2
		return Schema.new("Column",[columnid,tableid,datatype])
	end
end
class TermTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		TERMID=0
		RULEID=1
		TERM_POS=2
		TERM_TXT=3

	end
	@@SCHEMA = [Integer,Integer,Integer,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "TermTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::TERMID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		termid = Variable.new("termid",Integer)
		termid.position=0
		ruleid = Variable.new("ruleid",Integer)
		ruleid.position=1
		term_pos = Variable.new("term_pos",Integer)
		term_pos.position=2
		term_txt = Variable.new("term_txt",String)
		term_txt.position=3
		return Schema.new("Term",[termid,ruleid,term_pos,term_txt])
	end
end
class TableTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		TABLEID=0
		TABLENAME=1

	end
	@@SCHEMA = [Integer,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "TableTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::TABLEID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		tableid = Variable.new("tableid",Integer)
		tableid.position=0
		tablename = Variable.new("tablename",String)
		tablename.position=1
		return Schema.new("Table",[tableid,tablename])
	end
end
class PrimaryExpressionTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PRIMARYEXPRESSIONID=0
		TERMID=1
		P_POS=2
		P_TXT=3
		TYPE=4
		DATATYPE=5

	end
	@@SCHEMA = [Integer,Integer,Integer,String,String,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "PrimaryExpressionTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::PRIMARYEXPRESSIONID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		primaryexpressionid = Variable.new("primaryexpressionid",Integer)
		primaryexpressionid.position=0
		termid = Variable.new("termid",Integer)
		termid.position=1
		p_pos = Variable.new("p_pos",Integer)
		p_pos.position=2
		p_txt = Variable.new("p_txt",String)
		p_txt.position=3
		type = Variable.new("type",String)
		type.position=4
		datatype = Variable.new("datatype",String)
		datatype.position=5
		return Schema.new("PrimaryExpression",[primaryexpressionid,termid,p_pos,p_txt,type,datatype])
	end
end
class FactTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		FACTID=0
		PROGRAMID=1
		TABLENAME=2

	end
	@@SCHEMA = [Integer,Integer,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "FactTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::FACTID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		factid = Variable.new("factid",Integer)
		factid.position=0
		programid = Variable.new("programid",Integer)
		programid.position=1
		tablename = Variable.new("tablename",String)
		tablename.position=2
		return Schema.new("Fact",[factid,programid,tablename])
	end
end
class PredicateTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PREDICATEID=0
		TERMID=1
		PRED_POS=2
		PRED_TXT=3

	end
	@@SCHEMA = [Integer,Integer,Integer,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "PredicateTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::PREDICATEID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		predicateid = Variable.new("predicateid",Integer)
		predicateid.position=0
		termid = Variable.new("termid",Integer)
		termid.position=1
		pred_pos = Variable.new("pred_pos",Integer)
		pred_pos.position=2
		pred_txt = Variable.new("pred_txt",String)
		pred_txt.position=3
		return Schema.new("Predicate",[predicateid,termid,pred_pos,pred_txt])
	end
end
class MyIndexTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		INDEXID=0
		TABLEID=1
		INDX_POS=2

	end
	@@SCHEMA = [Integer,Integer,Integer]
	def initialize
		super(TableName.new(GLOBALSCOPE, "MyIndexTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::INDEXID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		indexid = Variable.new("indexid",Integer)
		indexid.position=0
		tableid = Variable.new("tableid",Integer)
		tableid.position=1
		indx_pos = Variable.new("indx_pos",Integer)
		indx_pos.position=2
		return Schema.new("MyIndex",[indexid,tableid,indx_pos])
	end
end
class ExpressionTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		EXPRESSIONID=0
		TERMID=1
		EXPR_POS=2
		EXPR_TEXT=3

	end
	@@SCHEMA = [Integer,Integer,Integer,String]
	def initialize
		super(TableName.new(GLOBALSCOPE, "ExpressionTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		programKey = Key.new(Field::EXPRESSIONID)
		index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
		@secondary[programKey] = index
	end
	def schema_of
		expressionid = Variable.new("expressionid",Integer)
		expressionid.position=0
		termid = Variable.new("termid",Integer)
		termid.position=1
		expr_pos = Variable.new("expr_pos",Integer)
		expr_pos.position=2
		expr_text = Variable.new("expr_text",String)
		expr_text.position=3
		return Schema.new("Expression",[expressionid,termid,expr_pos,expr_text])
	end
end
