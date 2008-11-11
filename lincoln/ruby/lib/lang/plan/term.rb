class Term
  include Comparable
  
	@@identifier = 0
		
	def initialize
		@program  = nil
		@rule     = nil
		@position = nil
		@identifier = "tid:" + @@identifier.to_s
		@@identifier += 1
	end
	
	attr_reader :identifier
	attr_accessor :location, :program, :rule, :position
	  	
  def ==(o) 
		if (o.class <= Term)
			return ((self<=>o) == 0)
		end
		false
	end
	
	def <=>(o)
    return (@identifier<=>(o.identifier))
	end
	
	def to_s
	  raise "subclass method for Term.to_s not defined"
  end
	
	def requires
	  raise "subclass method for Term.requires not defined"
  end
	
  def set(context, program, rule, position) 
	  @program = program
	  @rule = rule
	  @position = position
  end
  	
	def operator(context, input)
	  raise "subclass method for Term.operator not defined"
  end
end
