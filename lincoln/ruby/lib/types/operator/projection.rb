class Projection < Operator
	def initialize (context, predicate)
		super(context, predicate.program, predicate.rule)
		@accessors = []
		projection = []
		variables = predicate.schema.variables
		arguments = predicate.arguments

		variables.each_with_index do |var, i|
      @accessors << arguments[i].function
      projection << var
    end
		@schema = Schema.new(predicate.name, projection)
	end
	
	def to_s
		return "PROJECTION PREDICATE[" + schema.to_s + "]"
	end

  def evaluate(tuples)
#    require 'ruby-debug'; debugger if @predicate.name.nil?
		result = TupleSet.new(schema.name)
		tuples.each do |tuple|
			the_values = Array.new
			@accessors.each do |a|
			  if not (a.methods.include? "evaluate")
          raise "no evaluate method for tuple accessor" 
        end
			  the_values << a.evaluate(tuple)
		  end
			projection = Tuple.new(*the_values)
			projection.schema = @schema
			result << projection
		end
		return result
	end

	def schema
		@schema
	end

	def requires
		@schema.variables
	end
end
