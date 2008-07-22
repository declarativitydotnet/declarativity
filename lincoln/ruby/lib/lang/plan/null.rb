class Null < Value
	def initialize
		super(nil)
	end
	
	def to_s
		return "null"
	end
end
