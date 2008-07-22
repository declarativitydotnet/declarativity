class Cast < Expression 
	def initialize(type, expression)
		@type = type
		@expression = expression
	end
	
	def function
		return @expression.function
	end

	def to_s
		return "(" + @type.to_s + ")" + @expression.to_s
	end

	attr_reader :type, :expression

	def variables
		return @expression.variables
	end
end