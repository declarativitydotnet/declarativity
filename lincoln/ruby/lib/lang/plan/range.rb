class Range < Expression
	class FunctionCC
		def initialize(begin, end)
			@begin = begin
			@end = end
		end
		def test(t)
			return ((begin <=> t) <= 0 and (t <=> end) <= 0)
		end
  end
	
	class FunctionOC
		def initialize(begin, end)
			@begin = begin
			@end = end
		end
		def test(t)
			return ((begin <=> test) < 0 and (test <=> end) <= 0)
		end
	end
	
	class FunctionCO
		def initialize(begin, end)
			@begin = begin
			@end = end
		end
		def test(t)
			return ((begin <=> test) <= 0 and (test <=> end) < 0)
		end
  end	
	
	class FunctionOO
		def initialize(begin, end)
			@begin = begin
			@end = end
		end
		def test(t)
			return ((begin <=> test) < 0 and (test <=> end) < 0)
		end
  end
	
	
	class Operator 
	  CC='[]',
	  OC='(]',
		CO='[)',
    OO='()'
  end 

	def initialize(oper, begin, end)
		@oper = oper
		@begin = begin
		@end = end
	end

	def type
		begin.type
	end
	
	def to_s
		case @oper
		when Operator.CC
		  return "[" + begin + ", " + end + "]"
		when Operator.OC
			return "(" + begin + ", " + end + "]"
		when Operator.CO
			return "[" + begin + ", " + end + ")"
		when Operator.OO
			return "(" + begin + ", " + end + ")"
		end
    raise "RANGE ERROR"
	end

	def variables
		variables = Array.new
		variables += @begin.variables
		variables += @end.variables
		return variables
	end

	def function
    startFn = begin.function
		endFn   = end.function

    e_lam = lambda do |t| 
		 case @oper
	   when Operator.CC
	     return FunctionCC.new(start,end)
     when Operator.OC
       return FunctionOC.new(start,end)
     when Operator.CO
       return FunctionCO.new(start,end)
     when Operator.OO
       return FunctionOO.new(start,end)
     end
     raise "range error"
	  end
	  
	  tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      return e_lam.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      return Method
    end
    retval = tmpClass.new
    return retval
  end
end