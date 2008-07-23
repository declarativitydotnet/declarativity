require 'lib/lang/plan/expression'
class Boolean < Expression 
  def initialize(oper, lhs, rhs)
    @oper = oper
    @lhs = lhs
    @rhs = rhs
  end

  def type
    return Boolean
  end

  def to_s
    return "(" + @lhs.to_s + " " + 
    @oper.to_s + " " + @rhs.to_s + ")";
  end

  def variables
    variables = Array.new
    @lhs.variables.map{|v| variables << v}
    if (!@rhs.nil?) then
      @rhs.variables.map{|v| variables << v}
    end
    return variables;
  end

  def function
    doit = lambda do |t|
      lhs = @lhs.function.evaluate(t)
      rhs = @rhs.function.evaluate(t) unless @rhs.nil?
      return lhs.send(@oper, rhs) unless @rhs.nil?
      return nil
      # what about unary ops, i.e. NOT??
      # what about non-rubyisms like IN??
    end
    tmpClass = Class.new(TupleFunction)

    tmpClass.send :define_method, :evaluate do |tuple|
      return doit.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      return Boolean 
    end
    retval = tmpClass.new
    return retval
  end
end
