class Value < Expression
	def initialize(value)
		@value = value
	end
	
	attr_reader :value
	
	def expr_type
		return (@value.nil? ? nil : @value.class)
	end
	
  def to_s
    # test INFINITY!!
		return value.to_s
	end

  def variables
    ## This seems weird
		return Array.new
	end

  def function()
    # Ruby MetaProgramming-Fu!
    # the lambda's here make sure that the local state, i.e. @value, is 
    # in a closure, so when these functions are called, they'll remember 
    # that state
    
    e_lam = lambda do 
      return @value
    end
    
    r_lam = lambda do
      return @value.class
    end
    
    tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      e_lam.call
    end
    tmpClass.send :define_method, :returnType do 
      r_lam.call
    end
    return tmpClass.new
  end
end
