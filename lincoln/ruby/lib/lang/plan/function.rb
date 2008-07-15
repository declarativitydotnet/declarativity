class Function < Term 
  class Field
    PROGRAM = 0
    RULE = 1
    POSITION = 2
    NAME = 3
    OBJECT = 4
  end
  
	class TableFunction < ObjectTable 
		@@PRIMARY_KEY = Key.new(0,1,2)
		
		@@SCHEMA =  [String, String, Integer, String, Function]
      # String.class,    // program name
      # String.class,    // rule name
      # Integer.class,   // position
      # String.class,    // function name
      # Function.class   // function object

		def initialize
			super(TableName.new(GLOBALSCOPE, "function"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
		end
		
		def insert(tuple)
			object = tuple.value(Field.OBJECT)
			raise UpdateException, "Predicate object null" if object.nil?
			
			object.program   = tuple.value(Field.PROGRAM)
			object.rule      = (String) tuple.value(Field.RULE)
			object.position  = (Integer) tuple.value(Field.POSITION)
			return super(tuple)
		end
	end

	def initialize(function, predicate) 
		@function = function
		@predicate = predicate
	end

	def operator(input)
		return Function.new(@function, @predicate);
	end

	def requires
		@predicate.requires
	end

	def set(program, rule, position)
		@predicate.set(program, rule, position)
		$program.tfunction.force(Tuple.new(program, rule, position, function.name, self))
	end

	def to_s 
		@function.name + "(" + @predicate.to_s + ")"
	end
	
	def predicate
		@predicate;
	end
end
