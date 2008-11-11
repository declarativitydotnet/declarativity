require 'lib/types/table/object_table'
require 'lib/lang/plan/term'
require 'lib/types/table/key'
require 'lib/types/operator/selection_op'
require 'lib/lang/plan/object_from_catalog'
require 'lib/lang/parse/schema'
class SelectionTerm < Term 	
  def initialize(bool)
    super()
    @predicate = bool
  end

  def to_s
    return predicate.to_s
  end

  def requires
    return predicate.variables
  end

  attr_reader :predicate

  def operator(context, input)
    return SelectionOp.new(context, self, input)
  end

  def set(context, program, rule, position) 
    super(context,program,rule,position)
    context.catalog.table(SelectionTable.table_name).force(Tuple.new(program, rule, position, self))
  end
end
