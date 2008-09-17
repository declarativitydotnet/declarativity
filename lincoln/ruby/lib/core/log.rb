class Log < ObjectTable
	
	@@PRIMARY_KEY = Key.new()
	
	class Type
	  INFO = 0 
	  WARNING = 1 
	  ERROR = 2
  end
	
	class Field
	  CLOCK = 0
	  PROGRAM = 1
	  RULE = 2
	  TYPE = 3 
	  MESSAGE = 4
  end
  
	@@SCHEMA = [Integer,String,String,String,String]
    # Long.class,    // Clock value 
    # String.class,  // Program name
    # String.class,  // Rule name
    # Enum.class,    // Message type
    # String.class   // Log message

	def initialize(stream)
		super(TableName.new(GLOBALSCOPE, "log"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
		@stream = stream
	end
	
  def insert(tuple)
		log =     "CLOCK[" + tuple.value(Field::CLOCK) + "], ";
		log       += "PROGRAM[" + tuple.value(Field::PROGRAM) + "], ";
		log       += "RULE[" + tuple.value(Field::RULE) + "], ";
		log       += "TYPE[" + tuple.value(Field::TYPE) + "], ";
		log       += tuple.value(Field::MESSAGE) + "\n";
		
		@stream << log
		return super.insert(tuple);
	end
end
