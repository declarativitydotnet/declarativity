class Alias < Variable
	def initialize(name, field, type)
		super(name, type)
		@position(field.to_i)
	end
	
	def to_s
		return super + " := $" + position()
	end
end
