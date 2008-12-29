class FunctionOp < Operator 
	def initialize(context, function, predicate)
		super(context, predicate.program, predicate.rule)
		@function = function
		@predicate = predicate
	end

  def evaluate(tuples)
#    # require 'ruby-debug'; debugger
		result = @function.insert(tuples, nil)
		result.each { |t| t.schema = @predicate.schema.clone }
		return result
  end

	def requires
		@predicate.requires
	end

	def schema()
		@predicate.schema.clone
	end

  def to_s
		return @function.name + "(" + predicate.to_s + ")"
	end
end
