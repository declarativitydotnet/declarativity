class NewClass < Expression
	def initialize(type)
		@type = type
	end

  attr_reader :type

	def to_s
		value = "new " + @constructor.to_s.match(/,*#(.*)</) + "("
		return value + ")" if (@arguments.size == 0)
		value += @arguments[0].to_s
		(1..@arguments.size).each { |i| value += ", " + @arguments[i]}
		return value + ")"
	end
	
	def variables
		variables = Hash.new
		@arguments.each { |arg| variables += arg.variables }
		return variables
	}
	
	attr_accessor :constructor, :arguments
	
	def function
		argFunctions = array.new
		@arguments.each { |a| argFunctions << a.function }
		# the following is dodgy -- JMH
		doit = lambda do |t| 
		  arguments = Array.new
		  argFunctions.each_with_index { |a_fun, i| arguments[i] = a_fun.call(t)}
		  return @constructor(arguments)
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