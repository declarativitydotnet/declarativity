
class ArbitraryExpression < Expression

  def initialize(expr,variables)
	@expr = expr.gsub(/\b([A-Z])/,'v\1')
	@variables = Array.new
	variables.each do |v|
		@variables << v
		
	end
  end

  def expr_type
    return nil
  end

  def to_s
    return "(" + @expr + ")"
  end

  def variables
	@variables
  end

  def function
    elam = lambda do |t|

	subexpr = ''
	@variables.each do |v|	
		if t.schema.contains(v) then
			# substitution is stupid: how many times are we gonna parse this thing??
			# instead, take advantage of rubiismo:
			subexpr = subexpr + "v"+v.name + " = "+t.value(v.name).to_s+"\n"
		else
			# I guess we have a problem if we have unbound variables...
			raise
		end	
	end

	subexpr = subexpr + @expr
	return eval(subexpr)
    end
    
    tlam = lambda do
      return expr_type
    end
    
    tmpClass = Class.new(TupleFunction)

    tmpClass.send :define_method, :evaluate do |tuple|
      return elam.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      return tlam.call
    end
    retval = tmpClass.new
    return retval
  end
end
