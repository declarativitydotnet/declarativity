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
		POSITION=2
		TEXT=3

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
		position = Variable.new("position",Integer)
		position.position=2
		text = Variable.new("text",String)
		text.position=3
		return Schema.new("Term",[termid,ruleid,position,text])
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
		POSITION=2
		TEXT=3
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
		position = Variable.new("position",Integer)
		position.position=2
		text = Variable.new("text",String)
		text.position=3
		type = Variable.new("type",String)
		type.position=4
		datatype = Variable.new("datatype",String)
		datatype.position=5
		return Schema.new("PrimaryExpression",[primaryexpressionid,termid,position,text,type,datatype])
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
		POSITION=2
		TEXT=3

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
		position = Variable.new("position",Integer)
		position.position=2
		text = Variable.new("text",String)
		text.position=3
		return Schema.new("Predicate",[predicateid,termid,position,text])
	end
end
class MyIndexTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		INDEXID=0
		TABLEID=1
		POSITION=2

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
		position = Variable.new("position",Integer)
		position.position=2
		return Schema.new("MyIndex",[indexid,tableid,position])
	end
end
class ExpressionTable < ObjectTable
	@@PRIMARY_KEY = Key.new(0)
	class Field
		EXPRESSIONID=0
		TERMID=1
		POSITION=2
		TEXT=3

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
		position = Variable.new("position",Integer)
		position.position=2
		text = Variable.new("text",String)
		text.position=3
		return Schema.new("Expression",[expressionid,termid,position,text])
	end
end
