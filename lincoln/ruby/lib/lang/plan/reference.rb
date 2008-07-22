class Reference < Expression
	def initialize(type, name)
		@type = type
		@name = name
	end
	
	def to_s
		@name
	end

  attr_reader :type
end