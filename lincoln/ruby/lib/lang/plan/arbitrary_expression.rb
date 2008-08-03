
class ArbitraryExpression < Expression

  def initialize(expr,variables)
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
    return "(" + @expr + ")"
  end

  def variables
	@variables
  end

  def function
    elam = lambda do |t|

	subexpr = ''
	@variables.each do |v|	
		#print "\tcheck "+v.to_s+"("+v.name+")\n"
		if t.schema.contains(v) then
			# uh-oh
			#print "sub into ("+@expr+") <- "+t.value(v.name).to_s+"\n"
			#regexp = "([^a-zA-Z0-9])"+v.name
			#replace = '$1'+t.value(v.name).to_s
			#@expr = @expr.gsub(regexp,replace)
			#print "got expr "+@expr+"\n"
			#print "reg "+regexp+"\n"

			# substitution is stupid: how many times are we gonna parse this thing??
			# instead, take advantage of rubiismo:
			subexpr = subexpr + v.name + " = "+t.value(v.name).to_s+"\n"
		else
			# I guess we have a problem if we have unbound variables...
			raise
		end	
		# now, in rubyland, terms with first characters uppercase are constants.  the opposite is true in overlog.  who cares?
		# well, because (I assume) of how constants are scoped, if we call evaluate() multiple times on a single object
		# the interpreter is going to complain that we're redefining constants.		
		#subexpr = subexpr + @expr
	end

	subexpr = subexpr + @expr
	#print "expr is "+subexpr+"\n"
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
