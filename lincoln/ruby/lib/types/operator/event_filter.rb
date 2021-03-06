require 'lib/types/operator/operator'
require 'lib/types/function/tuple_function'
class EventFilter < Operator
	class Filter < TupleFunction
		def initialize(position, function)
			@position = position
			@function = function
		end

		def evaluate(tuple)
			fvalue = function.evaluate(tuple)
			tvalue = tuple.values[@position]
			
			return (fvalue == tvalue || ((fvalue <=> tvalue) == 0))
		end

		def returnType
			return @function.returnType
		end
	end
	
	def initialize (context, predicate)
		super(context, predicate.program, predicate.rule)
		@predicate = predicate
		@filters = Array.new
		
		predicate.each do |arg|
			#puts predicate.inspect
			#print "POSITION: "+arg.position.to_s+"\n"
			# require 'ruby-debug'
#			debugger if arg.nil? or arg.position.nil?
			raise unless arg.position >= 0
			@filters << Filter.new(arg.position, arg.function) unless arg.class <= Variable
	  end
	end
	
	def new_pf(context, predicate, filter)
		super(context, predicate.program, predicate.rule)
		@predicate = predicate
		@filters = [filter]
  end
  
  def evaluate(tuples)
		return tuples if filters.size() == 0
		result = TupleSet.new(tuples.name)
		tuples.each do |tuple|
			valid = true
			@filters.each do |filter|
				valid = filter.evaluate(tuple)
				break if not valid
			end
			result << tuple if valid
		end
		return result
  end

	def requires
		@predicate.requires
	end

	def schema
		@predicate.schema.clone
	end

	def toString
		return "EVENT FILTER " + predicate.to_s + ": FILTERS " + @filters.to_s
	end
	
	def filters
		@filters.size()
	end
end
