class Clock < ObjectTable 
	@@PRIMARY_KEY = Key.new(0)
	
	class Field
	  LOCATION = 0
	  CLOCK = 1
  end
  
	@@SCHEMA = [String, Integer] # location, clockvalue
	
	def initialize(location)
		super(TableName.new(GLOBALSCOPE, "clock"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		@location = location;
		@clock = -1
	end
	
	def current()
		@clock
	end
	
	def insert_tup(tuple)
		time = tuple.value(Field::CLOCK)
		if (time < @clock) then
			throw UpdateException, "Invalid clock time " +  time.to_s + " current clock value = " + @clock.to_s
		end
		@clock = time
		return super(tuple)
	end
	
	def time(t)
		me = TupleSet.new(name)
		me << Tuple.new(@location, t)
		return me
	end
end
