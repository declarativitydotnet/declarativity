require 'lib/lang/plan/object_from_catalog'
class Assignment < Term
	def initialize(variable, value) 
		@variable = variable
		@value = value
	end
	
	def to_s
		return @variable.to_s + " := " + @value.to_s
	end

	def requires
		return @value.variables
	end
	
	attr_reader :variable, :value

  def operator(input)
		return Assign.new(self, input)
	end

	def set(program, rule, position)
		Compiler.assignment.force(Tuple.new(program, rule, position, self))
	end
end
