require 'lib/types/table/object_table'
require 'lib/lang/parse/compiler_mixins'
class GlobalCatalogTable < ObjectTable
  @@classes = Hash.new
  GLOBALSCOPE = 'global'
  def GlobalCatalogTable.classes
    @@classes.keys
  end
end

class FunctionTable < GlobalCatalogTable
  include FunctionTableMixin if defined? FunctionTableMixin
  @@PRIMARY_KEY = Key.new(0,1,2)
  class Field
    PROGRAM=0
    RULE=1
    POSITION=2
    NAME=3
    OBJECT=4
  end
  @@SCHEMA = [String,String,Integer,String,String]
  @@TABLENAME = TableName.new(GLOBALSCOPE, "function")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? FunctionTableMixin and FunctionTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    GLOBALSCOPE

  end
  def FunctionTable.pkey

    @@PRIMARY_KEY

  end
  def FunctionTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    rule = Variable.new("rule",String, 1,nil)
    position = Variable.new("position",Integer, 2,nil)
    name = Variable.new("name",String, 3,nil)
    object = Variable.new("object",String, 4,nil)
    return Schema.new("Function",[program,rule,position,name,object])
  end

  def FunctionTable.table_name
    @@TABLENAME
  end
end

class IndexTable < GlobalCatalogTable
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
  @@TABLENAME = TableName.new(GLOBALSCOPE, "index")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? IndexTableMixin and IndexTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    GLOBALSCOPE

  end
  def IndexTable.pkey

    @@PRIMARY_KEY

  end
  def IndexTable.schema

    @@SCHEMA

  end
  def schema_of
    tablename = Variable.new("tablename",TableName, 0,nil)
    key = Variable.new("key",Key, 1,nil)
    type = Variable.new("type",TableType, 2,nil)
    classname = Variable.new("classname",String, 3,nil)
    object = Variable.new("object",String, 4,nil)
    return Schema.new("Index",[tablename,key,type,classname,object])
  end

  def IndexTable.table_name
    @@TABLENAME
  end
end

class OperatorTable < GlobalCatalogTable
  include OperatorTableMixin if defined? OperatorTableMixin
  @@PRIMARY_KEY = Key.new(2)
  class Field
    PROGRAM=0
    RULE=1
    ID=2
    SELECTIVITY=3
  end
  @@SCHEMA = [String,String,String,Float]
  @@TABLENAME = TableName.new(GLOBALSCOPE, "operator")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? OperatorTableMixin and OperatorTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    GLOBALSCOPE

  end
  def OperatorTable.pkey

    @@PRIMARY_KEY

  end
  def OperatorTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    rule = Variable.new("rule",String, 1,nil)
    id = Variable.new("id",String, 2,nil)
    selectivity = Variable.new("selectivity",Float, 3,nil)
    return Schema.new("Operator",[program,rule,id,selectivity])
  end

  def OperatorTable.table_name
    @@TABLENAME
  end
end

require 'lib/types/table/object_table'
require 'lib/lang/parse/compiler_mixins'
class BootstrapCatalogTable < ObjectTable
  @@classes = Hash.new
  BOOTSTRAPSCOPE = 'bootstrap'
  def BootstrapCatalogTable.classes
    @@classes.keys
  end
end

class CompilerTable < BootstrapCatalogTable
  include CompilerTableMixin if defined? CompilerTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    NAME=0
    OWNER=1
    FILE=2
    PROGRAM=3
  end
  @@SCHEMA = [String,String,String,String]
  @@TABLENAME = TableName.new(BOOTSTRAPSCOPE, "compiler")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? CompilerTableMixin and CompilerTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    BOOTSTRAPSCOPE

  end
  def CompilerTable.pkey

    @@PRIMARY_KEY

  end
  def CompilerTable.schema

    @@SCHEMA

  end
  def schema_of
    name = Variable.new("name",String, 0,nil)
    owner = Variable.new("owner",String, 1,nil)
    file = Variable.new("file",String, 2,nil)
    program = Variable.new("program",String, 3,nil)
    return Schema.new("Compiler",[name,owner,file,program])
  end

  def CompilerTable.table_name
    @@TABLENAME
  end
end

class QueryTable < BootstrapCatalogTable
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
  @@TABLENAME = TableName.new(BOOTSTRAPSCOPE, "query")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? QueryTableMixin and QueryTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    BOOTSTRAPSCOPE

  end
  def QueryTable.pkey

    @@PRIMARY_KEY

  end
  def QueryTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    rule = Variable.new("rule",String, 1,nil)
    public = Variable.new("public",Integer, 2,nil)
    delete = Variable.new("delete",Integer, 3,nil)
    event = Variable.new("event",String, 4,nil)
    input = Variable.new("input",TableName, 5,nil)
    output = Variable.new("output",TableName, 6,nil)
    object = Variable.new("object",String, 7,nil)
    return Schema.new("Query",[program,rule,public,delete,event,input,output,object])
  end

  def QueryTable.table_name
    @@TABLENAME
  end
end

require 'lib/types/table/object_table'
require 'lib/lang/parse/compiler_mixins'
class CompilerCatalogTable < ObjectTable
  @@classes = Hash.new
  COMPILERSCOPE = 'compiler'
  def CompilerCatalogTable.classes
    @@classes.keys
  end
end

class MyAssignmentTable < CompilerCatalogTable
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
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myAssignment")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyAssignmentTableMixin and MyAssignmentTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyAssignmentTable.pkey

    @@PRIMARY_KEY

  end
  def MyAssignmentTable.schema

    @@SCHEMA

  end
  def schema_of
    assignmentid = Variable.new("assignmentid",Integer, 0,nil)
    termid = Variable.new("termid",Integer, 1,nil)
    assign_pos = Variable.new("assign_pos",Integer, 2,nil)
    lhs = Variable.new("lhs",String, 3,nil)
    assign_txt = Variable.new("assign_txt",String, 4,nil)
    return Schema.new("MyAssignment",[assignmentid,termid,assign_pos,lhs,assign_txt])
  end

  def MyAssignmentTable.table_name
    @@TABLENAME
  end
end

class MyColumnTable < CompilerCatalogTable
  include MyColumnTableMixin if defined? MyColumnTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    COLUMNID=0
    TABLEID=1
    COL_POS=2
    DATATYPE=3
  end
  @@SCHEMA = [Integer,Integer,Integer,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myColumn")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyColumnTableMixin and MyColumnTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyColumnTable.pkey

    @@PRIMARY_KEY

  end
  def MyColumnTable.schema

    @@SCHEMA

  end
  def schema_of
    columnid = Variable.new("columnid",Integer, 0,nil)
    tableid = Variable.new("tableid",Integer, 1,nil)
    col_pos = Variable.new("col_pos",Integer, 2,nil)
    datatype = Variable.new("datatype",String, 3,nil)
    return Schema.new("MyColumn",[columnid,tableid,col_pos,datatype])
  end

  def MyColumnTable.table_name
    @@TABLENAME
  end
end

class MyExpressionTable < CompilerCatalogTable
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
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myExpression")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyExpressionTableMixin and MyExpressionTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyExpressionTable.pkey

    @@PRIMARY_KEY

  end
  def MyExpressionTable.schema

    @@SCHEMA

  end
  def schema_of
    expressionid = Variable.new("expressionid",Integer, 0,nil)
    termid = Variable.new("termid",Integer, 1,nil)
    arg_pos = Variable.new("arg_pos",Integer, 2,nil)
    expr_pos = Variable.new("expr_pos",Integer, 3,nil)
    expr_text = Variable.new("expr_text",String, 4,nil)
    return Schema.new("MyExpression",[expressionid,termid,arg_pos,expr_pos,expr_text])
  end

  def MyExpressionTable.table_name
    @@TABLENAME
  end
end

class MyFactTable < CompilerCatalogTable
  include MyFactTableMixin if defined? MyFactTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    FACTID=0
    TERMID=1
    TABLENAME=2
  end
  @@SCHEMA = [Integer,Integer,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myFact")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyFactTableMixin and MyFactTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyFactTable.pkey

    @@PRIMARY_KEY

  end
  def MyFactTable.schema

    @@SCHEMA

  end
  def schema_of
    factid = Variable.new("factid",Integer, 0,nil)
    termid = Variable.new("termid",Integer, 1,nil)
    tablename = Variable.new("tablename",String, 2,nil)
    return Schema.new("MyFact",[factid,termid,tablename])
  end

  def MyFactTable.table_name
    @@TABLENAME
  end
end

class MyIndexTable < CompilerCatalogTable
  include MyIndexTableMixin if defined? MyIndexTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    INDEXID=0
    TABLEID=1
    COL_POS=2
  end
  @@SCHEMA = [Integer,Integer,Integer]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myIndex")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyIndexTableMixin and MyIndexTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyIndexTable.pkey

    @@PRIMARY_KEY

  end
  def MyIndexTable.schema

    @@SCHEMA

  end
  def schema_of
    indexid = Variable.new("indexid",Integer, 0,nil)
    tableid = Variable.new("tableid",Integer, 1,nil)
    col_pos = Variable.new("col_pos",Integer, 2,nil)
    return Schema.new("MyIndex",[indexid,tableid,col_pos])
  end

  def MyIndexTable.table_name
    @@TABLENAME
  end
end

class MyPredicateTable < CompilerCatalogTable
  include MyPredicateTableMixin if defined? MyPredicateTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    PREDICATEID=0
    TERMID=1
    PRED_POS=2
    PRED_TXT=3
    EVENT_MOD=4
    NOTIN=5
  end
  @@SCHEMA = [Integer,Integer,Integer,String,String,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myPredicate")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyPredicateTableMixin and MyPredicateTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyPredicateTable.pkey

    @@PRIMARY_KEY

  end
  def MyPredicateTable.schema

    @@SCHEMA

  end
  def schema_of
    predicateid = Variable.new("predicateid",Integer, 0,nil)
    termid = Variable.new("termid",Integer, 1,nil)
    pred_pos = Variable.new("pred_pos",Integer, 2,nil)
    pred_txt = Variable.new("pred_txt",String, 3,nil)
    event_mod = Variable.new("event_mod",String, 4,nil)
    notin = Variable.new("notin",String, 5,nil)
    return Schema.new("MyPredicate",[predicateid,termid,pred_pos,pred_txt,event_mod,notin])
  end

  def MyPredicateTable.table_name
    @@TABLENAME
  end
end

class MyPrimaryExpressionTable < CompilerCatalogTable
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
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myPrimaryExpression")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyPrimaryExpressionTableMixin and MyPrimaryExpressionTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyPrimaryExpressionTable.pkey

    @@PRIMARY_KEY

  end
  def MyPrimaryExpressionTable.schema

    @@SCHEMA

  end
  def schema_of
    primaryexpressionid = Variable.new("primaryexpressionid",Integer, 0,nil)
    expressionid = Variable.new("expressionid",Integer, 1,nil)
    p_pos = Variable.new("p_pos",Integer, 2,nil)
    p_txt = Variable.new("p_txt",String, 3,nil)
    type = Variable.new("type",String, 4,nil)
    datatype = Variable.new("datatype",String, 5,nil)
    return Schema.new("MyPrimaryExpression",[primaryexpressionid,expressionid,p_pos,p_txt,type,datatype])
  end

  def MyPrimaryExpressionTable.table_name
    @@TABLENAME
  end
end

class MyProgramTable < CompilerCatalogTable
  include MyProgramTableMixin if defined? MyProgramTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    PROGRAMID=0
    OWNER=1
    PROGRAM_NAME=2
  end
  @@SCHEMA = [Integer,String,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myProgram")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyProgramTableMixin and MyProgramTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyProgramTable.pkey

    @@PRIMARY_KEY

  end
  def MyProgramTable.schema

    @@SCHEMA

  end
  def schema_of
    programid = Variable.new("programid",Integer, 0,nil)
    owner = Variable.new("owner",String, 1,nil)
    program_name = Variable.new("program_name",String, 2,nil)
    return Schema.new("MyProgram",[programid,owner,program_name])
  end

  def MyProgramTable.table_name
    @@TABLENAME
  end
end

class MyRuleTable < CompilerCatalogTable
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
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myRule")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyRuleTableMixin and MyRuleTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyRuleTable.pkey

    @@PRIMARY_KEY

  end
  def MyRuleTable.schema

    @@SCHEMA

  end
  def schema_of
    ruleid = Variable.new("ruleid",Integer, 0,nil)
    programid = Variable.new("programid",Integer, 1,nil)
    rulename = Variable.new("rulename",String, 2,nil)
    public = Variable.new("public",Integer, 3,nil)
    delete = Variable.new("delete",Integer, 4,nil)
    return Schema.new("MyRule",[ruleid,programid,rulename,public,delete])
  end

  def MyRuleTable.table_name
    @@TABLENAME
  end
end

class MySelectionTable < CompilerCatalogTable
  include MySelectionTableMixin if defined? MySelectionTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    SELECTIONID=0
    TERMID=1
    SELECT_POS=2
    SELECT_TXT=3
  end
  @@SCHEMA = [Integer,Integer,Integer,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "mySelection")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MySelectionTableMixin and MySelectionTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MySelectionTable.pkey

    @@PRIMARY_KEY

  end
  def MySelectionTable.schema

    @@SCHEMA

  end
  def schema_of
    selectionid = Variable.new("selectionid",Integer, 0,nil)
    termid = Variable.new("termid",Integer, 1,nil)
    select_pos = Variable.new("select_pos",Integer, 2,nil)
    select_txt = Variable.new("select_txt",String, 3,nil)
    return Schema.new("MySelection",[selectionid,termid,select_pos,select_txt])
  end

  def MySelectionTable.table_name
    @@TABLENAME
  end
end

class MyTableTable < CompilerCatalogTable
  include MyTableTableMixin if defined? MyTableTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    TABLEID=0
    TABLENAME=1
    WATCH=2
  end
  @@SCHEMA = [Integer,String,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myTable")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyTableTableMixin and MyTableTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyTableTable.pkey

    @@PRIMARY_KEY

  end
  def MyTableTable.schema

    @@SCHEMA

  end
  def schema_of
    tableid = Variable.new("tableid",Integer, 0,nil)
    tablename = Variable.new("tablename",String, 1,nil)
    watch = Variable.new("watch",String, 2,nil)
    return Schema.new("MyTable",[tableid,tablename,watch])
  end

  def MyTableTable.table_name
    @@TABLENAME
  end
end

class MyTableFunctionTable < CompilerCatalogTable
  include MyTableFunctionTableMixin if defined? MyTableFunctionTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    TABLEFUNID=0
    TERMID=1
    FUNCTION=2
    NESTED_PREDICATE_ID=3
  end
  @@SCHEMA = [Integer,Integer,String,Integer]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myTableFunction")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyTableFunctionTableMixin and MyTableFunctionTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyTableFunctionTable.pkey

    @@PRIMARY_KEY

  end
  def MyTableFunctionTable.schema

    @@SCHEMA

  end
  def schema_of
    tablefunid = Variable.new("tablefunid",Integer, 0,nil)
    termid = Variable.new("termid",Integer, 1,nil)
    function = Variable.new("function",String, 2,nil)
    nested_predicate_id = Variable.new("nested_predicate_id",Integer, 3,nil)
    return Schema.new("MyTableFunction",[tablefunid,termid,function,nested_predicate_id])
  end

  def MyTableFunctionTable.table_name
    @@TABLENAME
  end
end

class MyTermTable < CompilerCatalogTable
  include MyTermTableMixin if defined? MyTermTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    TERMID=0
    RULEID=1
    TERM_POS=2
    TERM_TXT=3
  end
  @@SCHEMA = [Integer,Integer,Integer,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "myTerm")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? MyTermTableMixin and MyTermTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def MyTermTable.pkey

    @@PRIMARY_KEY

  end
  def MyTermTable.schema

    @@SCHEMA

  end
  def schema_of
    termid = Variable.new("termid",Integer, 0,nil)
    ruleid = Variable.new("ruleid",Integer, 1,nil)
    term_pos = Variable.new("term_pos",Integer, 2,nil)
    term_txt = Variable.new("term_txt",String, 3,nil)
    return Schema.new("MyTerm",[termid,ruleid,term_pos,term_txt])
  end

  def MyTermTable.table_name
    @@TABLENAME
  end
end

class AssignmentTable < CompilerCatalogTable
  include AssignmentTableMixin if defined? AssignmentTableMixin
  @@PRIMARY_KEY = Key.new(0,1)
  class Field
    PROGRAM=0
    RULE=1
    POSITION=2
    OBJECT=3
  end
  @@SCHEMA = [String,String,Integer,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "assignment")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? AssignmentTableMixin and AssignmentTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def AssignmentTable.pkey

    @@PRIMARY_KEY

  end
  def AssignmentTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    rule = Variable.new("rule",String, 1,nil)
    position = Variable.new("position",Integer, 2,nil)
    object = Variable.new("object",String, 3,nil)
    return Schema.new("Assignment",[program,rule,position,object])
  end

  def AssignmentTable.table_name
    @@TABLENAME
  end
end

class FactTable < CompilerCatalogTable
  include FactTableMixin if defined? FactTableMixin
  @@PRIMARY_KEY = Key.new
  class Field
    PROGRAM=0
    TABLENAME=1
    TUPLE=2
  end
  @@SCHEMA = [String,TableName,Tuple]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "fact")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? FactTableMixin and FactTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def FactTable.pkey

    @@PRIMARY_KEY

  end
  def FactTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    tablename = Variable.new("tablename",TableName, 1,nil)
    tuple = Variable.new("tuple",Tuple, 2,nil)
    return Schema.new("Fact",[program,tablename,tuple])
  end

  def FactTable.table_name
    @@TABLENAME
  end
end

class PredicateTable < CompilerCatalogTable
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
  @@TABLENAME = TableName.new(COMPILERSCOPE, "predicate")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? PredicateTableMixin and PredicateTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def PredicateTable.pkey

    @@PRIMARY_KEY

  end
  def PredicateTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    rule = Variable.new("rule",String, 1,nil)
    position = Variable.new("position",Integer, 2,nil)
    event = Variable.new("event",String, 3,nil)
    object = Variable.new("object",String, 4,nil)
    return Schema.new("Predicate",[program,rule,position,event,object])
  end

  def PredicateTable.table_name
    @@TABLENAME
  end
end

class ProgramTable < CompilerCatalogTable
  include ProgramTableMixin if defined? ProgramTableMixin
  @@PRIMARY_KEY = Key.new(0)
  class Field
    PROGRAM=0
    OWNER=1
    OBJECT=2
  end
  @@SCHEMA = [String,String,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "program")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? ProgramTableMixin and ProgramTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def ProgramTable.pkey

    @@PRIMARY_KEY

  end
  def ProgramTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    owner = Variable.new("owner",String, 1,nil)
    object = Variable.new("object",String, 2,nil)
    return Schema.new("Program",[program,owner,object])
  end

  def ProgramTable.table_name
    @@TABLENAME
  end
end

class RuleTable < CompilerCatalogTable
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
  @@TABLENAME = TableName.new(COMPILERSCOPE, "rule")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? RuleTableMixin and RuleTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def RuleTable.pkey

    @@PRIMARY_KEY

  end
  def RuleTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    name = Variable.new("name",String, 1,nil)
    is_public = Variable.new("is_public",String, 2,nil)
    is_delete = Variable.new("is_delete",String, 3,nil)
    object = Variable.new("object",String, 4,nil)
    return Schema.new("Rule",[program,name,is_public,is_delete,object])
  end

  def RuleTable.table_name
    @@TABLENAME
  end
end

class SelectionTable < CompilerCatalogTable
  include SelectionTableMixin if defined? SelectionTableMixin
  @@PRIMARY_KEY = Key.new(0,1,2)
  class Field
    PROGRAM=0
    RULE=1
    POSITION=2
    OBJECT=3
  end
  @@SCHEMA = [String,String,Integer,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "selection")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? SelectionTableMixin and SelectionTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def SelectionTable.pkey

    @@PRIMARY_KEY

  end
  def SelectionTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    rule = Variable.new("rule",String, 1,nil)
    position = Variable.new("position",Integer, 2,nil)
    object = Variable.new("object",String, 3,nil)
    return Schema.new("Selection",[program,rule,position,object])
  end

  def SelectionTable.table_name
    @@TABLENAME
  end
end

class WatchTable < CompilerCatalogTable
  include WatchTableMixin if defined? WatchTableMixin
  @@PRIMARY_KEY = Key.new(0,1,2)
  class Field
    PROGRAM=0
    TABLENAME=1
    MODIFIER=2
  end
  @@SCHEMA = [String,String,String]
  @@TABLENAME = TableName.new(COMPILERSCOPE, "watch")
  @@classes[self] = 1
  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    if defined? WatchTableMixin and WatchTableMixin.methods.include? 'initialize_mixin'
       initialize_mixin(context) 
    end
  end

  def field(name)

    eval('Field::'+name)

  end
  def scope

    COMPILERSCOPE

  end
  def WatchTable.pkey

    @@PRIMARY_KEY

  end
  def WatchTable.schema

    @@SCHEMA

  end
  def schema_of
    program = Variable.new("program",String, 0,nil)
    tablename = Variable.new("tablename",String, 1,nil)
    modifier = Variable.new("modifier",String, 2,nil)
    return Schema.new("Watch",[program,tablename,modifier])
  end

  def WatchTable.table_name
    @@TABLENAME
  end
end

