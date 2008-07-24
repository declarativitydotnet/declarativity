class Function < Operator 
	def initialize(function, predicate)
		super(predicate.program, predicate.rule)
		@function = function
		@predicate = predicate
	end

  def evaluate(tuples)
		result = @function.insert(tuples, null)
		result.each { |t| t.schema = predicate.schema.clone }
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
