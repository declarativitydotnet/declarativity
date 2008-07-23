class Projection < Operator
	def initialize (predicate)
		super(predicate.program, predicate.rule)
		@predicate = predicate

  	@accessors = Array.new		
		predicate.each do |arg| 
			@accessors << arg.function
		end
	end
	
	def to_s
		return "PROJECTION PREDICATE[" + predicate.to_s + "]"
	end

  def evaluate(tuples)
		result = TupleSet.new(@predicate.name)
		tuples.each do |tuple|
			values = Array.new
			@accessors.each { |a| values << a.evaluate(tuple) }
			projection = Tuple.new(*values)
			projection.schema = @predicate.schema
			result << projection
		end
		return result
	end

	def schema
		@predicate.schema
	end

	def requires
		@predicate.requires
	end
	
	attr_reader :predicate
end
