require 'lib/lang/plan/predicate'
require 'lib/types/operator/function_op.rb'
class Function < Predicate 
  class Field
    PROGRAM = 0
    RULE = 1
    POSITION = 2
    NAME = 3
    OBJECT = 4
  end

	def initialize(function, predicate) 
#	  # require 'ruby-debug'; debugger
		super(predicate.notin, predicate.name, predicate.event, predicate.arguments)
		@function = function
	end

	def operator(context, input)
		return FunctionOp.new(context, @function, self);
	end

	def requires
		@predicate.requires
	end

	def set(context, program, rule, position)
		super(context, program, rule, position)
		context.catalog.table(FunctionTable.table_name).force(Tuple.new(program, rule, position, @function.name, self))
	end

	def to_s 
		@function.name.to_s + "(" + super + ")"
	end
	
	attr_reader :predicate
end
