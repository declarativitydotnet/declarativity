class Clause 
  include Comparable
  def initialize(location)
		@location = location
	end
	
	attr_reader :location

	def <=>(o)
		return location.<=>(o.location)
	end

	def to_s
	  raise "Clause.to_s needs to be subclassed"
  end
	
	def set(program) 
	  raise "Clause.set needs to be subclassed"
  end
}
