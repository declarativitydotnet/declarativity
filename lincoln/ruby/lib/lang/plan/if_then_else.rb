class IfThenElse < Expression
	def IfThenElse(type, ifexpr, thenexpr, elseexpr)
		@type = type
		@ifexpr = ifexpr
		@thenexpr = thenexpr
		@elseexpr = elseexpr
	end

	def to_s
		return ifexpr.to_s + " ? " +  thenexpr.to_s + " : " + elseexpr.to_s
	end

  attr_reader :type

  def variables
		variables = Array.new
		ifexpr.variables.each { |v| variables << v }
		thenexpr.variables.each { |v| variables << v }
		elseexpr.variables.each { |b| variables << v }
		return variables
	end

  def function()
    test = @ifexpr.function
    thencase = thenexpr.function
    elsecase = elseexpr.function

    e_lam = lambda do |t|
      return (text.evaluate(t) == true ? thencase.evaluate(t) : elsecase.evaluate(t))
    end

    r_lam = lambda do
      return @type
    end

    tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      e_lam.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      r_lam.call
    end
    return tmpClass.new
  end
end
