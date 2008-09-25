class Assign < Operator
  def initialize (assignment, input)
		super(assignment.program, assignment.rule)
		@assignment = assignment
		@schema = input.clone

		#@schema.append(@assignment.variable) unless @schema.includes? @assignment.variable 
		@schema << @assignment.variable unless @schema.contains @assignment.variable 
	end
	
	def to_s
		@assignment.to_s
	end

  def evaluate(tuples)
		variable = @assignment.variable
		function = @assignment.value.function
#		require 'ruby-debug'; debugger
		deltas = TupleSet.new(tuples.name)
		tuples.each do |tuple|
			delta = tuple.clone
			delta.set_value(variable, function.evaluate(delta))
			deltas << delta
		end
		return deltas
	end
	
	attr_reader :schema

	def requires
		@assignment.requires
	end
end
