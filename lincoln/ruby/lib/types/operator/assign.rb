class Assign < Operator
  def initialize (assignment, input)
		super(assignment.program, assignment.rule)
		this.assignment = assignment
		this.schema = input.clone

		this.schema.append(this.assignment.variable) unless this.schema.includes? this.assignment.variable 
	end
	
	def to_s
		@assignment.to_s
	end

  def evaluate(tuples)
		variable = assignment.variable
		function = assignment.value.function
		deltas = TupleSet.new(tuples.name)
		tuples.each do |tuple|
			delta = tuple.clone
			delta.value(variable, function.evaluate(delta))
			deltas << delta
		end
		return deltas
	end
	
	attr_reader :schema

	def requires
		@assignment.requires
	end
end
