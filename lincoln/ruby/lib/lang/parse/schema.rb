require 'lib/types/table/object_table'
require 'lib/lang/parse/catalog_mixins'
class AssignmentTable < ObjectTable
include AssignmentTableMixin if defined? AssignmentTableMixin
	@@PRIMARY_KEY = Key.new(0,1)
	class Field
		PROGRAM=0
		RULE=1
		POSITION=2
	end
	@@SCHEMA = [String,String,Integer]

	def initialize
		super(TableName.new(GLOBALSCOPE, "AssignmentTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? AssignmentTableMixin and AssignmentTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		rule = Variable.new("rule",String)
		rule.position=1
		position = Variable.new("position",Integer)
		position.position=2
		return Schema.new("Assignment",[program,rule,position])
	end
end

class CompilerTable < ObjectTable
include CompilerTableMixin if defined? CompilerTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		NAME=0
		OWNER=1
		FILE=2
		PROGRAM=3
	end
	@@SCHEMA = [String,String,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "CompilerTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? CompilerTableMixin and CompilerTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		name = Variable.new("name",String)
		name.position=0
		owner = Variable.new("owner",String)
		owner.position=1
		file = Variable.new("file",String)
		file.position=2
		program = Variable.new("program",String)
		program.position=3
		return Schema.new("Compiler",[name,owner,file,program])
	end
end

class FactTable < ObjectTable
include FactTableMixin if defined? FactTableMixin
	@@PRIMARY_KEY = Key.new
	class Field
		PROGRAM=0
		TABLENAME=1
		TUPLE=2
	end
	@@SCHEMA = [String,TableName,Tuple]

	def initialize
		super(TableName.new(GLOBALSCOPE, "FactTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? FactTableMixin and FactTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		tablename = Variable.new("tablename",TableName)
		tablename.position=1
		tuple = Variable.new("tuple",Tuple)
		tuple.position=2
		return Schema.new("Fact",[program,tablename,tuple])
	end
end

class IndexTable < ObjectTable
include IndexTableMixin if defined? IndexTableMixin
	@@PRIMARY_KEY = Key.new(0,1)
	class Field
		TABLENAME=0
		KEY=1
		TYPE=2
		CLASSNAME=3
		OBJECT=4
	end
	@@SCHEMA = [TableName,Key,TableType,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "IndexTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? IndexTableMixin and IndexTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		tablename = Variable.new("tablename",TableName)
		tablename.position=0
		key = Variable.new("key",Key)
		key.position=1
		type = Variable.new("type",TableType)
		type.position=2
		classname = Variable.new("classname",String)
		classname.position=3
		object = Variable.new("object",String)
		object.position=4
		return Schema.new("Index",[tablename,key,type,classname,object])
	end
end

class MyColumnTable < ObjectTable
include MyColumnTableMixin if defined? MyColumnTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		COLUMNID=0
		TABLEID=1
		DATATYPE=2
	end
	@@SCHEMA = [Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyColumnTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyColumnTableMixin and MyColumnTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		columnid = Variable.new("columnid",Integer)
		columnid.position=0
		tableid = Variable.new("tableid",Integer)
		tableid.position=1
		datatype = Variable.new("datatype",String)
		datatype.position=2
		return Schema.new("MyColumn",[columnid,tableid,datatype])
	end
end

class MyExpressionTable < ObjectTable
include MyExpressionTableMixin if defined? MyExpressionTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		EXPRESSIONID=0
		TERMID=1
		EXPR_POS=2
		EXPR_TEXT=3
	end
	@@SCHEMA = [Integer,Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyExpressionTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyExpressionTableMixin and MyExpressionTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

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
		return Schema.new("MyExpression",[expressionid,termid,expr_pos,expr_text])
	end
end

class MyFactTable < ObjectTable
include MyFactTableMixin if defined? MyFactTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		FACTID=0
		PROGRAMID=1
		TABLENAME=2
	end
	@@SCHEMA = [Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyFactTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyFactTableMixin and MyFactTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		factid = Variable.new("factid",Integer)
		factid.position=0
		programid = Variable.new("programid",Integer)
		programid.position=1
		tablename = Variable.new("tablename",String)
		tablename.position=2
		return Schema.new("MyFact",[factid,programid,tablename])
	end
end

class MyIndexTable < ObjectTable
include MyIndexTableMixin if defined? MyIndexTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		INDEXID=0
		TABLEID=1
		INDX_POS=2
	end
	@@SCHEMA = [Integer,Integer,Integer]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyIndexTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyIndexTableMixin and MyIndexTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

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

class MyPredicateTable < ObjectTable
include MyPredicateTableMixin if defined? MyPredicateTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PREDICATEID=0
		TERMID=1
		PRED_POS=2
		PRED_TXT=3
	end
	@@SCHEMA = [Integer,Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyPredicateTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyPredicateTableMixin and MyPredicateTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

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
		return Schema.new("MyPredicate",[predicateid,termid,pred_pos,pred_txt])
	end
end

class MyPrimaryExpressionTable < ObjectTable
include MyPrimaryExpressionTableMixin if defined? MyPrimaryExpressionTableMixin
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
		super(TableName.new(GLOBALSCOPE, "MyPrimaryExpressionTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyPrimaryExpressionTableMixin and MyPrimaryExpressionTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

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
		return Schema.new("MyPrimaryExpression",[primaryexpressionid,termid,p_pos,p_txt,type,datatype])
	end
end

class MyProgramTable < ObjectTable
include MyProgramTableMixin if defined? MyProgramTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PROGRAM=0
		OWNER=1
		OBJECT=2
	end
	@@SCHEMA = [Integer,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyProgramTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyProgramTableMixin and MyProgramTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",Integer)
		program.position=0
		owner = Variable.new("owner",String)
		owner.position=1
		object = Variable.new("object",String)
		object.position=2
		return Schema.new("MyProgram",[program,owner,object])
	end
end

class MyRuleTable < ObjectTable
include MyRuleTableMixin if defined? MyRuleTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		RULEID=0
		PROGRAMID=1
		RULENAME=2
		PUBLIC=3
		DELETE=4
	end
	@@SCHEMA = [Integer,Integer,String,Integer,Integer]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyRuleTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyRuleTableMixin and MyRuleTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		ruleid = Variable.new("ruleid",Integer)
		ruleid.position=0
		programid = Variable.new("programid",Integer)
		programid.position=1
		rulename = Variable.new("rulename",String)
		rulename.position=2
		public = Variable.new("public",Integer)
		public.position=3
		delete = Variable.new("delete",Integer)
		delete.position=4
		return Schema.new("MyRule",[ruleid,programid,rulename,public,delete])
	end
end

class MyTableTable < ObjectTable
include MyTableTableMixin if defined? MyTableTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		TABLEID=0
		TABLENAME=1
	end
	@@SCHEMA = [Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyTableTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyTableTableMixin and MyTableTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		tableid = Variable.new("tableid",Integer)
		tableid.position=0
		tablename = Variable.new("tablename",String)
		tablename.position=1
		return Schema.new("MyTable",[tableid,tablename])
	end
end

class MyTermTable < ObjectTable
include MyTermTableMixin if defined? MyTermTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		TERMID=0
		RULEID=1
		TERM_POS=2
		TERM_TXT=3
	end
	@@SCHEMA = [Integer,Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyTermTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyTermTableMixin and MyTermTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

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
		return Schema.new("MyTerm",[termid,ruleid,term_pos,term_txt])
	end
end

class OperatorTable < ObjectTable
include OperatorTableMixin if defined? OperatorTableMixin
	@@PRIMARY_KEY = Key.new(2)
	class Field
		PROGRAM=0
		RULE=1
		ID=2
		SELECTIVITY=3
	end
	@@SCHEMA = [String,String,String,Float]

	def initialize
		super(TableName.new(GLOBALSCOPE, "OperatorTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? OperatorTableMixin and OperatorTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		rule = Variable.new("rule",String)
		rule.position=1
		id = Variable.new("id",String)
		id.position=2
		selectivity = Variable.new("selectivity",Float)
		selectivity.position=3
		return Schema.new("Operator",[program,rule,id,selectivity])
	end
end

class PredicateTable < ObjectTable
include PredicateTableMixin if defined? PredicateTableMixin
	@@PRIMARY_KEY = Key.new(0,1,2)
	class Field
		PROGRAM=0
		RULE=1
		POSITION=2
		EVENT=3
		OBJECT=4
	end
	@@SCHEMA = [String,String,Integer,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "PredicateTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? PredicateTableMixin and PredicateTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		rule = Variable.new("rule",String)
		rule.position=1
		position = Variable.new("position",Integer)
		position.position=2
		event = Variable.new("event",String)
		event.position=3
		object = Variable.new("object",String)
		object.position=4
		return Schema.new("Predicate",[program,rule,position,event,object])
	end
end

class ProgramTable < ObjectTable
include ProgramTableMixin if defined? ProgramTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PROGRAM=0
		OWNER=1
		OBJECT=2
	end
	@@SCHEMA = [String,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "ProgramTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? ProgramTableMixin and ProgramTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		owner = Variable.new("owner",String)
		owner.position=1
		object = Variable.new("object",String)
		object.position=2
		return Schema.new("Program",[program,owner,object])
	end
end

class QueryTable < ObjectTable
include QueryTableMixin if defined? QueryTableMixin
	@@PRIMARY_KEY = Key.new
	class Field
		PROGRAM=0
		RULE=1
		PUBLIC=2
		DELETE=3
		EVENT=4
		INPUT=5
		OUTPUT=6
		OBJECT=7
	end
	@@SCHEMA = [String,String,Integer,Integer,String,TableName,TableName,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "QueryTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? QueryTableMixin and QueryTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		rule = Variable.new("rule",String)
		rule.position=1
		public = Variable.new("public",Integer)
		public.position=2
		delete = Variable.new("delete",Integer)
		delete.position=3
		event = Variable.new("event",String)
		event.position=4
		input = Variable.new("input",TableName)
		input.position=5
		output = Variable.new("output",TableName)
		output.position=6
		object = Variable.new("object",String)
		object.position=7
		return Schema.new("Query",[program,rule,public,delete,event,input,output,object])
	end
end

class RuleTable < ObjectTable
include RuleTableMixin if defined? RuleTableMixin
	@@PRIMARY_KEY = Key.new(0,1)
	class Field
		PROGRAM=0
		NAME=1
		IS_PUBLIC=2
		IS_DELETE=3
		OBJECT=4
	end
	@@SCHEMA = [String,String,String,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "RuleTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? RuleTableMixin and RuleTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		name = Variable.new("name",String)
		name.position=1
		is_public = Variable.new("is_public",String)
		is_public.position=2
		is_delete = Variable.new("is_delete",String)
		is_delete.position=3
		object = Variable.new("object",String)
		object.position=4
		return Schema.new("Rule",[program,name,is_public,is_delete,object])
	end
end

class SelectionTable < ObjectTable
include SelectionTableMixin if defined? SelectionTableMixin
	@@PRIMARY_KEY = Key.new(0,1,2)
	class Field
		PROGRAM=0
		RULE=1
		POSITION=2
		OBJECT=3
	end
	@@SCHEMA = [String,String,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "SelectionTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? SelectionTableMixin and SelectionTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		rule = Variable.new("rule",String)
		rule.position=1
		position = Variable.new("position",Integer)
		position.position=2
		object = Variable.new("object",String)
		object.position=3
		return Schema.new("Selection",[program,rule,position,object])
	end
end

class WatchTable < ObjectTable
include WatchTableMixin if defined? WatchTableMixin
	@@PRIMARY_KEY = Key.new(0,1,2)
	class Field
		PROGRAM=0
		TABLENAME=1
		MODIFIER=2
	end
	@@SCHEMA = [String,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "WatchTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? WatchTableMixin and WatchTableMixin.methods.include? 'initialize_mixin'
			 then initialize_mixin 
		end
	end

	def field(name)

		eval('Field::'+name)

	end
	def scope

		GLOBALSCOPE

	end
	def pkey

		@@PRIMARY_KEY

	end
	def schema

		@@SCHEMA

	end
	def schema_of
		program = Variable.new("program",String)
		program.position=0
		tablename = Variable.new("tablename",String)
		tablename.position=1
		modifier = Variable.new("modifier",String)
		modifier.position=2
		return Schema.new("Watch",[program,tablename,modifier])
	end
end

