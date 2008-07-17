require 'lib/types/basic/tuple_set'
require 'lib/types/operator/operator'

class SelectionOp < Operator
  def initialize(selection, input)
    super(selection.program, selection.rule)
    @selection = selection
    @schema = input.clone
  end
  
  def to_s
    return "SELECTION [" + @selection.to_s + "]"
  end

  def evaluate(tuples)
    result = TupleSet.new(tuples.name)
    filter = @selection.predicate.function
    tuples.each do |t|
        result << t   if filter.evaluate(t)
    end
    return result
  end

  attr_reader :schema

  def requires
    @selection.predicate.variables
  end
end