class Location < Variable
	def initialize(name, type)
		super(name, type)
	end
	
	def clone
		clone = Location.new(name, type)
		clone.position(@position)
		return clone
	end
	
	def to_s
		return "@" + super
	end
end
