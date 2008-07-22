class Math < Expression
	
	public final static String ADD    = "+";
	public final static String INC    = "++";
	public final static String MINUS  = "-";
	public final static String DEC    = "--";
	public final static String TIMES  = "*";
	public final static String DIVIDE = "/";
	public final static String MOD    = "%";
	public final static String POW    = "^";
	public final static String LSHIFT = "<<";
	public final static String RSHIFT = ">>";


	
  def initialize(oper, lhs, rhs) {
		@oper = oper
		@lhs = lhs
		@rhs = rhs
	end

	def type
		return lhs.type
	end
	
	def to_s
		return "(" + @lhs + " " + @oper + " " + @rhs + ")"
	end

	def variables
		variables = Array.new
		@lhs.variables.each { |v| variables << v }
		@rhs.variables.each { |v| variables << v}  unless (rhs.nil?) 
		return variables;
	end

	def function
		return BasicMath.function(oper, lhs.function, rhs.function)
	end
end
