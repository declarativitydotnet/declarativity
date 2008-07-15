class Term
  include Comparable
  
	@@identifier = 0
		
	def initialize
		@program  = nil
		@rule     = nil
		@position = nil
		@identifer = "tid:" + @@identifier.to_s
		@@identifier += 1
	end
	
	def identifier
		@identifer
	end
	
	def location
	  @location
  end
  
	def location=(location)
		@location = location
	end
		
  def ==(o) 
		if (o.class <= Term)
			return ((self<=>o) == 0)
		end
		false
	end
	
	def <=>(o)
		@identifer.<=>(o.identifer)
	end
	
	def program
		@program
	end
	
	def rule
		@rule
	end
	
	def position
		@position;
	end
	
	def to_s
	  raise "subclass method for Term.to_s not defined"
  end
	
	def requires
	  raise "subclass method for Term.requires not defined"
  end
	
  def set(program, rule, position) 
	  raise "subclass method for Term.set not defined"
  end
  	
	def operator(input)
	  raise "subclass method for Term.operator not defined"
  end
end
