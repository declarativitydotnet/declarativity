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

  def initialize(context, program, rule)
    @context = context
    @identifier = newId
    @program = program
    @rule = rule
    me = Tuple.new(OperatorTable.name, program, rule, @identifier, nil, self)
    # catalog tuple generated in Runtime.initialize
    # context.catalog.table(OperatorTable.table_name).force(me)
  end
  
  attr_reader :identifier, :program
  attr_accessor :rule

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
