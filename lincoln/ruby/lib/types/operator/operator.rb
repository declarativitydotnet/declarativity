require 'lib/types/table/table_name'
require 'lib/types/table/object_table'
require 'lib/types/table/key'
require 'lib/types/basic/type_list'
require 'lib/lang/parse/schema'

class Operator
  include Comparable

  @@id = 0
  def newId 
    identifier = "Operator:" + @@id.to_s
    @@id += 1
    return identifier
  end 

  @@table = OperatorTable.new

  def initialize(program, rule)
    @identifier = newId
    @program = program
    @rule = rule
    me = Tuple.new(@@table.name, program, rule, @identifier, nil, self)
    @@table.insert_tup(me)
  end
  
  attr_reader :identifier

  def <=>(o)
    return (@identifier <=> o.identifier)
  end

  def to_s
    #raise "subclass method for Operator.to_s not defined"
	return "some Operator"
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
