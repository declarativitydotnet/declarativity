class AntiScanJoin < Join
	def initialize (context, predicate, input)
		super(context, predicate, input)
		@table = context.catalog.table(predicate.name)
	end

  def to_s
		return "ANTI NEST LOOP JOIN: PREDICATE[" + @predicate.to_s  + "]"
	end
	
  def evaluate(tuples)
		result = TupleSet.new("anti",nil)
		tuples.each do |outer|
			success = false
			@table.tuples.each do |inner|
				inner.schema = @predicate.schema
				if (validate(outer, inner) && !(outer.join(inner).nil?))
					success = true
					break
				end
			end
			if (!success) then result << outer end
		end
		return result
	end
end
