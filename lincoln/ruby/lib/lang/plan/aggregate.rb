
require 'lib/lang/plan/variable'

class Aggregate < Variable 
	@@STAR = '*'
	
	def initialize(name, function, type, position, location)
		super(name, type, position, location)
		@function = function
	end
	
	def clone
		Aggregate.new(@name, @function, type)
	end

	def to_s
		@function.to_s + "<" + super + ">"
	end
	
	def functionName
		@function
	end
	
	def function
    return AggregateFunction.function(self)
  end
end

