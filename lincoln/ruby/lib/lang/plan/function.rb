require 'lib/lang/plan/term'
class Function < Term 
  class Field
    PROGRAM = 0
    RULE = 1
    POSITION = 2
    NAME = 3
    OBJECT = 4
  end

	def initialize(function, predicate) 
		@function = function
		@predicate = predicate
	end

	def operator(context, input)
		return Function.new(context, @function, @predicate);
	end

	def requires
		@predicate.requires
	end

	def set(context, program, rule, position)
		super(context, program, rule, position)
		context.catalog.table(TableFunction.table_name).force(Tuple.new(program, rule, position, @function.name, self))
	end

	def to_s 
		@function.name + "(" + @predicate.to_s + ")"
	end
	
	attr_reader :predicate
end
