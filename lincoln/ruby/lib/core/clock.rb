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
	
	def insert(tuple)
		time = tuple.value(Field.CLOCK)
		if (time < @clock) then
			throw UpdateException, "Invalid clock time " +  time.to_s + " current clock value = " + this.clock.to_s
		end
		@clock = time
		return super.insert(tuple)
	end
	
	def time(t)
		me = TupleSet.new(name)
		me << Tuple(@location, t)
		return me
	end
end
