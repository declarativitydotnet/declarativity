
class ArbitraryExpression < Expression

  def initialize(expr,variables)
    @expr = expr
    @variables = Array.new
    variables.each do |v|
     # @variables << v
      if v.class == Variable then
	unless v.name == ""
        	@expr = @expr.gsub(/\b#{v.name}/,'v'+v.name)
		@variables << v
	end
      else 
       @variables << v
      end
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
      @variables.each_with_index do |v, i|	
        if v.class == Variable
	  ##next if v.name == ""
          if t.schema.contains(v) then
            # substitution is stupid: how many times are we gonna parse this thing??
            # instead, take advantage of rubiismo:
            subexpr = subexpr + "v"+v.name + " = "+t.value(v.name).to_s+"\n"
          elsif defined?(@variables[i+1]) and @variables[i+1].class == Value
            # unbound variables had better belong to assignments
            # which show up as a Variable followed by a Value
            subexpr = subexpr + "v"+v.name + " = "+@variables[i+1].value.to_s+"\n"
          end
        elsif (v.class == Value)
          next
        else
          raise "unable to resolve a PrimaryExpression"
        end
      end
      subexpr = subexpr + @expr
      ##require 'ruby-debug'; debugger
      print "EXPR : #{subexpr}\n"
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
