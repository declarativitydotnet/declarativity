class Log < ObjectTable
	
	@@PRIMARY_KEY = Key.new()
	@@TABLENAME = TableName.new(GLOBALSCOPE, "log")
	
	class Type
	  INFO = 0 
	  WARNING = 1 
	  ERROR = 2
  end
	
	class Field
	  TYPE = 0
	  MESSAGE = 1
  end
  
	@@SCHEMA = [String,String]
    # Enum.class,    // Message type
    # String.class   // Log message

	def initialize(context, stream)
		super(context, @@TABLENAME, @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
		@stream = stream
	end
	
  def insert(tuple)
		log = "LOGTYPE [" + tuple.value(Field::TYPE) + "], "
		log += tuple.value(Field::MESSAGE) + "\n"
		
		@stream << log
		return super.insert(tuple)
	end
end
