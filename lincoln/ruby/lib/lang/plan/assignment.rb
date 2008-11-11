require 'lib/lang/plan/object_from_catalog'
require 'lib/types/operator/assign.rb'

#require 'lib/lang/compiler.rb'
class Assignment < Term
	def initialize(variable, value) 
		@variable = variable
		@value = value
	end
	
	def to_s
		return "ASSIGNMENT["+@variable.to_s + " := " + @value.to_s+"]"
	end

	def requires
		return @value.variables
	end
	
	attr_reader :variable, :value

  def operator(context, input)
		return Assign.new(context, self, input)
	end

	def set(context, program, rule, position)
	  super(context,program,rule,position)
		context.catalog.table(AssignmentTable.table_name).force(Tuple.new(program, rule, position, self))
	end

	# not sure if this is right
	def ==(o)
		if o.class <= Assignment
			return ((self<=>o) == 0)
		end	
		false
	end
	def <=>(o)
		return to_s <=> o.to_s
	end

end
