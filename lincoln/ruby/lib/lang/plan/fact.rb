require 'lib/lang/plan/clause'
require 'lib/types/table/hash_index'
class Fact < Clause	
	def initialize(location, name, arguments)
		super(location)
		@name = name
		# require 'ruby-debug'; debugger unless (@name.class <= TableName)
		@arguments = arguments
		@type = @arguments[0].class
	end
	
	def to_s
		value = @name.to_s + "("
		return value + ")" if (@arguments.size == 0)
		value += @arguments[0].to_s
		@arguments[1..@arguments.length].each { |a| value += ", "  + a.to_s }
		return value + ")."
	end

	def set(context, program) 
		values = Array.new
		@arguments.each do |argument|
		  if argument.class <= Expression
			  function = argument.function
        #			# require 'ruby-debug'; debugger
        #			if (@arguments[0].class == Value)
        values << function.evaluate(nil)
      else 
        values << argument
      # elsif (@arguments[0].class == ArbitraryExpression)
      #   values << function.evaluate(Tuple.new())
      end
		end
		
		unless @name.class <= TableName
		  # require 'ruby-debug'; debugger
	  end
		context.catalog.table(FactTable.table_name).force(Tuple.new(program, @name, Tuple.new(*values)))
	end

end
