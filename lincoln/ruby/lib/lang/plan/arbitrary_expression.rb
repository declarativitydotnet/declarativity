
class ArbitraryExpression < Expression

  def initialize(expr,variables)
    @expr = expr
    @variables = Array.new
    sides = expr.split(".")
    case sides.size 
    when 0
    when 1 
      # good
    when 2
      if variables.size != 1
        #raise("object.method can only be bound over one variable (object)")
      end
      @obj = sides[0]
      @method = sides[1]
    else	
      print "size #{sides.size}\n"
      print "expr #{expr}\n"
      raise("only one object.method per expression allowed")
    end

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
      unless @method.nil? 
        value = t.value(@variables[0].name)
        # if value.class == Program then
        if @variables.size > 1 then
          return value.send(@method,*@variables[1..@variables.size-1])		
        else
          return value.send(@method)
        end
        # end
      end
      @variables.each_with_index do |v, i|	
        if v.class == Variable
          if t.schema.contains(v) then
            # substitution is stupid: how many times are we gonna parse this thing??
            # instead, take advantage of rubiismo:
            value = t.value(v.name)
            # workaround
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
      ##print "EVAL: #{subexpr}\n"
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
