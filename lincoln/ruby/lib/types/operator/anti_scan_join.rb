class AntiScanJoin < Join
	def initialize (predicate, input)
		super(predicate, input)
		@table = Table.table(predicate.name)
	end

  def to_s
		return "ANTI NEST LOOP JOIN: PREDICATE[" + @predicate  + "]"
	end
	
  def evaluate(tuples)
		result = TupleSet.new
		tuples.each do |outer|
			success = false
			@table.tuples.each do |inner|
				inner.schema = this.predicate.schema
				if (validate(outer, inner) && !(outer.join(inner).nil?))
					success = true
					break
				end
			end
			if (!success) result << outer
		end
		return result
	end
end
