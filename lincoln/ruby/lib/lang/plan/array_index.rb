class ArrayIndex < Expression
  def initialize(array, index)
		# assert(array.type().isArray());
		@array = array
		@index = index
	end
	
  class type
    return @array.type.getComponentType
  end
	
	def to_s
		return "(" + @array.to_s + ")[" + @index.to_s + "]"
	end

	def variables
		return @array.variables
	end

	def function
	  doit = lambda do |t|
      return @array.function.call(tuple)[index]
    end
    tmpClass = Class.new(TupleFunction)

    tmpClass.send :define_method, :evaluate do |tuple|
      return doit.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      return type 
    end
    retval = tmpClass.new
    return retval
	end
end
