require 'lib/types/table/object_table'
require 'lib/lang/parse/compiler_mixins'
class MyAssignmentTable < ObjectTable
include MyAssignmentTableMixin if defined? MyAssignmentTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		ASSIGNMENTID=0
		TERMID=1
		ASSIGN_POS=2
		LHS=3
		ASSIGN_TXT=4
	end
	@@SCHEMA = [Integer,Integer,Integer,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyAssignment"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MyAssignmentTableMixin and MyAssignmentTableMixin.methods.include? 'initialize_mixin'
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
		assignmentid = Variable.new("assignmentid",Integer)
		assignmentid.position=0
		termid = Variable.new("termid",Integer)
		termid.position=1
		assign_pos = Variable.new("assign_pos",Integer)
		assign_pos.position=2
		lhs = Variable.new("lhs",String)
		lhs.position=3
		assign_txt = Variable.new("assign_txt",String)
		assign_txt.position=4
		return Schema.new("MyAssignment",[assignmentid,termid,assign_pos,lhs,assign_txt])
	end
end

class MyColumnTable < ObjectTable
include MyColumnTableMixin if defined? MyColumnTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		COLUMNID=0
		TABLEID=1
		COL_POS=2
		DATATYPE=3
	end
	@@SCHEMA = [Integer,Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyColumn"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		col_pos = Variable.new("col_pos",Integer)
		col_pos.position=2
		datatype = Variable.new("datatype",String)
		datatype.position=3
		return Schema.new("MyColumn",[columnid,tableid,col_pos,datatype])
	end
end

class MyExpressionTable < ObjectTable
include MyExpressionTableMixin if defined? MyExpressionTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		EXPRESSIONID=0
		TERMID=1
		ARG_POS=2
		EXPR_POS=3
		EXPR_TEXT=4
	end
	@@SCHEMA = [Integer,Integer,Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyExpression"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		arg_pos = Variable.new("arg_pos",Integer)
		arg_pos.position=2
		expr_pos = Variable.new("expr_pos",Integer)
		expr_pos.position=3
		expr_text = Variable.new("expr_text",String)
		expr_text.position=4
		return Schema.new("MyExpression",[expressionid,termid,arg_pos,expr_pos,expr_text])
	end
end

class MyFactTable < ObjectTable
include MyFactTableMixin if defined? MyFactTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		FACTID=0
		TERMID=1
		TABLENAME=2
	end
	@@SCHEMA = [Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyFact"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		termid = Variable.new("termid",Integer)
		termid.position=1
		tablename = Variable.new("tablename",String)
		tablename.position=2
		return Schema.new("MyFact",[factid,termid,tablename])
	end
end

class MyIndexTable < ObjectTable
include MyIndexTableMixin if defined? MyIndexTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		INDEXID=0
		TABLEID=1
		COL_POS=2
	end
	@@SCHEMA = [Integer,Integer,Integer]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyIndex"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		col_pos = Variable.new("col_pos",Integer)
		col_pos.position=2
		return Schema.new("MyIndex",[indexid,tableid,col_pos])
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
		EVENT_MOD=4
	end
	@@SCHEMA = [Integer,Integer,Integer,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyPredicate"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		event_mod = Variable.new("event_mod",String)
		event_mod.position=4
		return Schema.new("MyPredicate",[predicateid,termid,pred_pos,pred_txt,event_mod])
	end
end

class MyPrimaryExpressionTable < ObjectTable
include MyPrimaryExpressionTableMixin if defined? MyPrimaryExpressionTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PRIMARYEXPRESSIONID=0
		EXPRESSIONID=1
		P_POS=2
		P_TXT=3
		TYPE=4
		DATATYPE=5
	end
	@@SCHEMA = [Integer,Integer,Integer,String,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyPrimaryExpression"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		expressionid = Variable.new("expressionid",Integer)
		expressionid.position=1
		p_pos = Variable.new("p_pos",Integer)
		p_pos.position=2
		p_txt = Variable.new("p_txt",String)
		p_txt.position=3
		type = Variable.new("type",String)
		type.position=4
		datatype = Variable.new("datatype",String)
		datatype.position=5
		return Schema.new("MyPrimaryExpression",[primaryexpressionid,expressionid,p_pos,p_txt,type,datatype])
	end
end

class MyProgramTable < ObjectTable
include MyProgramTableMixin if defined? MyProgramTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		PROGRAMID=0
		OWNER=1
		PROGRAM_NAME=2
	end
	@@SCHEMA = [Integer,String,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MyProgram"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		programid = Variable.new("programid",Integer)
		programid.position=0
		owner = Variable.new("owner",String)
		owner.position=1
		program_name = Variable.new("program_name",String)
		program_name.position=2
		return Schema.new("MyProgram",[programid,owner,program_name])
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
		super(TableName.new(GLOBALSCOPE, "MyRule"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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

class MySelectionTable < ObjectTable
include MySelectionTableMixin if defined? MySelectionTableMixin
	@@PRIMARY_KEY = Key.new(0)
	class Field
		SELECTIONID=0
		TERMID=1
		SELECT_POS=2
		SELECT_TXT=3
	end
	@@SCHEMA = [Integer,Integer,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "MySelection"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		if defined? MySelectionTableMixin and MySelectionTableMixin.methods.include? 'initialize_mixin'
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
		selectionid = Variable.new("selectionid",Integer)
		selectionid.position=0
		termid = Variable.new("termid",Integer)
		termid.position=1
		select_pos = Variable.new("select_pos",Integer)
		select_pos.position=2
		select_txt = Variable.new("select_txt",String)
		select_txt.position=3
		return Schema.new("MySelection",[selectionid,termid,select_pos,select_txt])
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
		super(TableName.new(GLOBALSCOPE, "MyTable"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "MyTerm"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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

class AssignmentTable < ObjectTable
include AssignmentTableMixin if defined? AssignmentTableMixin
	@@PRIMARY_KEY = Key.new(0,1)
	class Field
		PROGRAM=0
		RULE=1
		POSITION=2
		OBJECT=3
	end
	@@SCHEMA = [String,String,Integer,String]

	def initialize
		super(TableName.new(GLOBALSCOPE, "assignment"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		object = Variable.new("object",String)
		object.position=3
		return Schema.new("Assignment",[program,rule,position,object])
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
		super(TableName.new(GLOBALSCOPE, "compiler"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "fact"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "index"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "operator"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "predicate"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "program"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "query"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "rule"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "selection"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
		super(TableName.new(GLOBALSCOPE, "watch"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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

