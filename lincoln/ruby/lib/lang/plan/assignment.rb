class Assignment < Term
	
	class AssignmentTable < ObjectTable
		@@PRIMARY_KEY = Key.new(0,1)
		
		class Field 
		  PROGRAM=1 
		  RULE=2 
		  POSITION=3 
		  OBJECT=4
	  end
		@@SCHEMA = [String, String, Integer, Assignment]
      # String.class,    // Program name
      # String.class,    // Rule name
      # Integer.class,   // Rule position
      # Assignment.class // Assignment object

		def initialize
			super(TableName.new(GLOBALSCOPE, "assignment"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
		end
		
		def insert(tuple)
			object = tuple.value(Field::OBJECT)
			raise UpdateException, "Assignment object nil" if object.nil?
			object.program   = tuple.value(Field::PROGRAM)
			object.rule      = tuple.value(Field::RULE)
			object.position  = tuple.value(Field::POSITION)
			return super.insert(tuple)
		end
		
		def delete(tuple)
			return super(tuple)
		end
	end
	
	def initialize(variable, value) 
		@variable = variable
		@value = value
	end
	
	def to_s
		return @variable.to_s + " := " + @value.to_s
	end

	def requires
		return @value.variables
	end
	
	attr_reader :variable, :value

  def operator(input)
		return Assign.new(self, input)
	end

	def set(program, rule, position)
		Compiler.assignment.force(Tuple.new(program, rule, position, self))
	end
end
