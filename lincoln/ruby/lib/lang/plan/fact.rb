require 'lib/lang/plan/clause'
require 'lib/types/table/hash_index'
class Fact < Clause	
	def initialize(location, name, arguments)
		super(location)
		@name = name
		@arguments = arguments
	end
	
	def to_s
		value = name + "("
		return value + ")" if (@arguments.size == 0)
		value += @arguments[0].to_s
		@arguments.each { |a| value += ", "  + a.to_s }
		return value + ")."
	end

	def set(context, program) 
		values = Array.new
		@arguments.each do |argument|
			function = argument.function
      values << function.evaluate(nil)
		end
		
		unless @name.class <= TableName or @name.class <= String
		  require 'ruby-debug'; debugger
	  end
		context.catalog.table(FactTable.table_name).force(Tuple.new(program, @name, Tuple.new(*values)))
	end

end
