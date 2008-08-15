
require 'lib/lang/plan/variable'

class Aggregate < Variable 
	@@STAR = '*'
	
	def initialize(name, function, type)
		super(name, type)
		print "init name = #{@name}\n"
		@function = function
	end
	
	def clone
		print "CLONE\n"
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

