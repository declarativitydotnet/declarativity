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
		#puts tuples.name
		tuples.each do |outer|

			if tuples.name.to_s.eql?("global::MyPrimaryExpression") then
				#require 'ruby-debug'; debugger
			end
			@index.lookup_kt(@lookupKey, outer).each do |inner| 
				if validate(outer, inner) then
					#puts "found a tup for " + @index.table.name.name.to_s
					inner.schema = @predicate.schema.clone
					jointup = outer.join(inner)
					result << jointup unless jointup.nil?
				end
			end
		end
                if result.size == 0 then
		   #require 'ruby-debug';debugger
                   #puts "no tups for " + @index.table.name.name
                end
		return result
	end
end
