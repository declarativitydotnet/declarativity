class Fact < Clause
	
	class FactTable < ObjectTable
		@@PRIMARY_KEY = Key.new()
		
		class Field 
		  PROGRAM=0
		  TABLENAME=1
		  TUPLE=2
	  end
		@@SCHEMA =  [String, TableName, Tuple]
      # String.class,    // Program name
      # TableName.class, // Table name
      # Tuple.class      // Tuple object

		def initialize
			super(TableName.new(GLOBALSCOPE, "fact"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
			programKey = Key.new(Field.PROGRAM)
			index = HashIndex.new(self, programKey, Index.Type.SECONDARY)
			@secondary.put(programKey, index)
		end
		
		def insert(tuple)
			super(tuple)
		end
		
		def delete(tuple)
			super(tuple)
		end
	end
	
	def initialize(location, name, arguments)
		super(location)
		@name = name
		@arguments = arguments
	end
	
	def to_s
		value = name + "("
		return value + ")" if (arguments.size == 0)
		value += arguments[0].to_s
		(1..arguments.size) { |i| value += ", "  + arguments[i] }
		return value + ")."
	end

	def set(program) 
		values = Array.new
		@arguments.each do |argument} 
			function = argument.function
      values << function.evaluate(nil)
		end
		
		Compiler.fact.force(Tuple.new(program, name, Tuple.new(values)))
	end

end
