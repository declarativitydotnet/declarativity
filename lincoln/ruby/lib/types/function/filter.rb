require 'lib/types/function/tuple_function'
class Filter < TupleFunction
  include Comparable 
  
	class Operator
	  F_NOT=1
	  F_AND=2 
	  F_OR=3 
	  F_EQ=4 
	  F_NEQ=5
	  F_LTHAN=6
	  F_GTHAN=7
	  F_LEQ=8
	  F_GEQ=9
  end
	
	def initialize(oper, lhs, rhs) 
		@oper = oper
		@lhs = lhs
		@rhs = rhs
	end

	def evaluate(tuple)
		case @oper
  	when Operator.F_NOT
		  return f_not(tuple)
		when Operator.F_AND
		  return f_and(tuple)
		when Operator.F_OR
		  return f_or(tuple)
		when Operator.F_EQ
		  return f_eq(tuple)
		when Operator.F_NEQ
		  return f_neq(tuple)
		when Operator.F_LTHAN
		  return f_lthan(tuple)
		when Operator.F_GTHAN
		  return f_gthan(tuple)
		when Operator.F_LEQ
		  return f_leq(tuple)
		when Operator.F_GEQ
		  return f_geq(tuple)
	  end

		# TODO log fatal error.
		return false;
	end
	

   # Indicates the boolean value of the comparable object.
   # @param c The comparable object.
   # @return The boolean value of the comparable object.
	def value(c)
		if c == null:	return false
		elsif (c.class <= Boolean && !c):	return false
		elsif (c.class <= Numeric && c.to_i == 0):	return false
    elsif (c.class <= String && c.to_s == ""): return false
		end
		return true
	end
	
	def  f_not(tuple)
		return !value(@lhs.evaluate(tuple))
	end
	
	def  f_and(tuple)
		return (value(@lhs.evaluate(tuple)) && value(@rhs.evaluate(tuple)))
	end
	
	def  f_or(tuple)
		return (value(@lhs.evaluate(tuple)) || value(@rhs.evaluate(tuple)))
	end
	
	def  f_eq(tuple)
		throw "FILTER: " + @lhs.evaluate(tuple).to_s + " == " + @rhs.evaluate(tuple).to_s
		return ((@lhs.evaluate(tuple) <=> @rhs.evaluate(tuple)) == 0)
	end
	
	def  f_neq(tuple)
		return ((@lhs.evaluate(tuple) <=> @rhs.evaluate(tuple)) != 0)
	end
	
	def  f_lthan(tuple)
		return ((@lhs.evaluate(tuple) <=> @rhs.evaluate(tuple)) < 0)
	end

	def  f_gthan(tuple)
		return ((@lhs.evaluate(tuple) <=> @rhs.evaluate(tuple)) > 0)
	end
	
	def  f_leq(tuple)
		return ((@lhs.evaluate(tuple) <=> @rhs.evaluate(tuple)) <= 0)
	end

	def  f_geq(tuple)
		return ((@lhs.evaluate(tuple) <=> @rhs.evaluate(tuple)) >= 0)
	end

	def returnType()
		Boolean
	end
	
end
