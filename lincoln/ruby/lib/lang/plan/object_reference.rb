class ObjectReference < Reference
	
  def initialize(object, field)
		super(field.class, object.to_s + "." + field.name)
		@object = object
		@field = field
	end

  attr_reader :field, :object

  def variables
		return @object.variables
	end
	
  def function
    objectFunction = @object.function
    e_lam = lambda do |t| 
		  instance = objectFunction.call(tuple)
		  return @field[instance]
	  end
	  
	  t_lam = lambda do
	    return type
    end
    
	  tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      return e_lam.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      return t_lam.call
    end
    retval = tmpClass.new
    return retval
  end
end
