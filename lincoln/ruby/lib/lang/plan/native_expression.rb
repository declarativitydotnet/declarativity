class NativeExpression < Expression

  def initialize(oper, lhs, rhs)
    @oper = oper
    @lhs = lhs
    @rhs = rhs
  end

  def expr_type
    return lhs.class
  end

  def to_s
    return "(" + @lhs.to_s + " " + @oper.to_s + " " + @rhs.to_s + ")"
  end

  def variables
    variables = Array.new
    @lhs.variables.each { |v| variables << v }
    @rhs.variables.each { |v| variables << v}  unless (rhs.nil?) 
    return variables;
  end

  def function
    elam = lambda do |t|
      lhs = @lhs.function.evaluate(t)
      rhs = @rhs.function.evaluate(t) unless @rhs.nil?
      return lhs.send(@oper, rhs) unless @rhs.nil?
      return nil
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
