require 'lib/lang/plan/function'

class Periodic < ObjectTable 
	
	class Scheduler < Function
		def initialize(schedule) 
			super("periodicScheduler", TypeList.new(Periodic.schema))
			@schedule = schedule
		end

		def insert(tuples, conflicts)
			@schedule =  TupleSet.new(@schedule.name)
			deltas   = TupleSet.new(name)
			tuples.each do |tuple|
				program = tuple.value(Periodic.Field::PROGRAM)
				time    = tuple.value(Periodic.Field::TIME)
				periodics = TupleSet.new(TableName.new(program, "periodic"))
				periodics << tuple.clone
				schedule << Tuple.new(time, program, periodics.name, periodics, nil)
				deltas << tuple.clone
			end
			
			@schedule.insert(schedule, conflicts) unless schedule.size <= 0
			return deltas
		end
	end
	
	@@PRIMARY_KEY = Key.new(0)
	@@TABLENAME = TableName.new(GLOBALSCOPE, "periodic")
	
	class Field 
	  IDENTIFIER = 0
	  PERIOD = 1
	  TTL = 2
	  TIME = 3
	  COUNT = 4
	  PROGRAM = 5
  end
  
	@@SCHEMA = [String, Integer, Integer, Integer, Integer, String]
    # String.class, // Identifier
    # Long.class,   // Period
    # Long.class,   // TTL
    # Long.class,   // Time
    # Long.class,   // Count
    # String.class  // Program

  # give class variable access to the Schedule nested class
	def Periodic.schema
	  @@SCHEMA
  end

  def Periodic.table_name
    @@TABLENAME
  end
  
	def initialize(context,schedule)
		super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		# context.catalog.register(Scheduler.new(schedule))
	end

	def insert
  	identifier = tuple.value(Field.IDENTIFIER)
		tuple.value(Field.IDENTIFIER, Runtime.idgen().to_s) if (identifier.nil?)
		return super.insert(tuple)
	end
  	
	def min
		curmin = exp(2,Bignum.size) - 1
		tuples.each do |current|
			time = current.value(Field::TIME)
			curmin = curmin < time  ? curmin : time
		end
		return curmin
	end
end
