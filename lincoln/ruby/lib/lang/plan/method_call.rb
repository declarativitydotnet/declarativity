class MethodCall < Expression
	def initialize(object, method, arguments)
		@object = object
		@method = method
		@arguments = arguments
	end

  # def type
  #   return method.getReturnType
  # end
	
	def to_s
		value = @method.to_s.match(/,*#(.*)</) + "("
		return @object.to_s + "." + @value + ")" if @arguments.length == 0

		value += @arguments[0].to_s
		(1..@arguments.size).each |i|
			value += ", " + @arguments[i]
		end
		return @object.toString() + "." + value + ")"
	end

	def variables
		variables = Hash.new
		@object.variables.each { |o| variables << o }
		@arguments.each { |arg|	arg.variables.each { |v| variables << v } }
		return variables
	end

	def function
	  objectFunction = @object.function
	  argFunctions = Array.new
	  @arguments.each { |a| argfunctions << a.function }
	  
	  # I think this is pretty broken; Java code was unclear to me. -- JMH
	  doit = lambda do |t|
	    instance = objectFunction.call(tuple)
	    arguments = Array.new
	    argFunctions.each_with_index do |argFunction, index|
	      arguments[index] = argFunction.call(tuple)
      end
      return @method.call(instance, arguments)
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
