require 'lib/types/operator/join'
class IndexJoin < Join
	
	def initialize (context, predicate, input, lookupKey, index)
		super(context, predicate, input)
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
  		# require 'ruby-debug'; debugger if outer.schema.name == 'clock'
			if tuples.name.to_s.eql?("global::MyPrimaryExpression") then
				## require 'ruby-debug'; debugger
			end
			#print "TEST #{outer}\n"
			@index.lookup_kt(@lookupKey, outer).each do |inner| 
				#print "TEST #{inner}\n"
				if validate(outer, inner) then
					#puts "found a tup for " + @index.table.name.name.to_s
## require 'ruby-debug'; debugger
					inner.schema = @predicate.schema.clone
					jointup = outer.join(inner)
					result << jointup unless jointup.nil?
				else 
					#nprint "NO tup for #{@index.table.name.name.to_s}\n"
					## require 'ruby-debug'; debugger
				end
			end
		end
                if result.size == 0 then
		   ## require 'ruby-debug';debugger
                   #puts "no tups for " + @index.table.name.name
                end
		return result
	end
end
