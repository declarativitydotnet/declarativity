class BasicQuery < Query
	
	def initialize(program, rule, isPublic, isDelete, event, head, body)
		super(program, rule, isPublic, isDelete, event, head)
		@aggregation = nil
		@body        = body
	end
	
	def to_s
		String   query = "Basic Query Rule " + rule + 
		               ": input " + input.to_s
		body.each |oper| 
			query += " -> " + oper.to_s
		end
		query += " -> " + output().to_s
		return query
	end

	def evaluate(in)
		if (@input.name != in.name) then
			throw P2RuntimeException, "Query expects input " + in.name + 
					                     " but got input tuples " + @input.name
		end
		
		tuples = TupleSet.new(@input.name)
		in.each do |tuple| 
			tuple = tuple.clone
			tuple.schema(in.schema.clone);
			tuples << (tuple);
		end
		
		body.each do |oper| 
			tuples = oper.evaluate(tuples)
		end
		
		return tuples;
	end
end
