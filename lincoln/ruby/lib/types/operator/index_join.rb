require 'lib/types/operator/join'
class IndexJoin < Join
	
	def initialize (predicate, input, lookupKey, index)
		super(predicate, input)
		@lookupKey = lookupKey
		@index = index
	end
	
	def to_s
		return "INDEX JOIN: PREDICATE[" + @predicate.to_s + "]"
	end
	
	def evaluate(tuples)
		result = TupleSet.new("ij_out", nil)
		tuples.each do |outer|
			@index.lookup_kt(@lookupKey, outer).each do |inner| 
				if validate(outer, inner) then
					inner.schema = @predicate.schema.clone
					jointup = outer.join(inner)
					result << jointup unless jointup.nil?
				end
			end
		end
		return result
	end
end
