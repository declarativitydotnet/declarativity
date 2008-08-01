
class ArbitraryExpression < Expression

  def initialize(expr,*variables)
    @expr = expr
	@variables = Array.new
	variables.each do |v|
		@variables << v
	end
  end

  def expr_type
    return nil
  end

  def to_s
    return @expr
  end

  def variables
	@variables
  end

  def function
    elam = lambda do |t|
	@variables.each do |v|	
		print "\tcheck "+v.to_s+"("+v.name+")\n"
		if t.schema.contains(v) then
			# uh-oh
			print "sub into ("+@expr+") <- "+t.value(v.name).to_s+"\n"
			@expr = @expr.gsub(v.name,t.value(v.name).to_s)
		else
			# I guess we have a problem if we have unbound variables...
			raise
		end	
	end
	return eval(@expr)
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
