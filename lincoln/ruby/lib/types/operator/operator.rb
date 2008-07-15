require 'lib/types/table/table_name'
require 'lib/types/table/object_table'
require 'lib/types/table/key'
require 'lib/types/basic/type_list'
class Operator
  include Comparable

  @@id = 0
  def newId 
    identifier = "Operator:" + @@id.to_s
    @@id += 1
    return identifier
  end 

  class OperatorTable < ObjectTable
    @@PRIMARY_KEY = Key.new(2)
    class Field
      PROGRAM = 0
      RULE = 1
      ID = 2
      SELECTIVITY = 3
      OBJECT = 4
    end

    @@SCHEMA = [
      String.class,    # Program Name
      String.class,    # Rule Name
      String.class,    # Operator ID
      Float.class,     # Selectivity
      Operator.class   # The Operator object
    ]

    def initialize
      super(TableName.new(GLOBALSCOPE, "operator"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
    end

    # def insert(tuple)
    #   return super(tuple)
    # end
  end

  @@table = OperatorTable.new

  def initialize(program, rule)
    @identifier = newId
    @program = program
    @rule = rule
    me = Tuple.new(@@table.name, program, rule, @identifier, nil, self)
    @@table.insert_tup(me)
  end
  
  def identifier
    @identifier
  end

  def <=>(o)
    return (@identifier <=> o.identifier)
  end

  def to_s
    raise "subclass method for Operator.to_s not defined"
  end

  def evaluate(tuples)
    raise "subclass method for Operator.evaluate(tuples) not defined"
  end

  def schema
    raise "subclass method for Operator.schema not defined"
  end

  def requires
    raise "subclass method for Operator.requires not defined"
  end
end