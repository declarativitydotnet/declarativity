class Query
  include Comparable

	class QueryTable < ObjectTable 
		@@PRIMARY_KEY = Key.new
		
		class Field
		  PROGRAM = 1
		  RULE = 2 
		  PUBLIC = 3 
		  DELETE = 4 
		  EVENT = 5
		  INPUT = 6 
		  OUTPUT = 7 
		  OBJECT = 8
	  end
	  
		@@SCHEMA = [String, String, Integer, Integer, String, TableName, TableName, Query]
      # String.class,     // Program name
      # String.class,     // Rule name
      # Boolean.class,    // Public query?
      # Boolean.class,    // Delete rule/query?
      # String.class,     // Event modifier
      # TableName.class,  // Input table name
      # TableName.class,  // Output table name
      # Query.class       // The query object

		def initialize
			super(TableName.new(GLOBALSCOPE, "query"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		end
		
  end
		
	def initialize(program, rule, isPublic, isDelete, input, output)
		@program = program
		@rule = rule
		@isPublic = isPublic
		@isDelete = isDelete
		@event  = input.event()
		@input = input
		@output = output
	  System.query.force(Tuple.new(@program, @rule, @isPublic, @isDelete, @event.to_s, @input.name, @output.name, self))
  end	
	
	attr_reader :event, :program, :rule, :isDelete, :isPublic, :input, :output
	
	def to_s
	  throw "Query.to_s must be subclassed"
  end
	
	def <=>(q)
		return self.object_id < q.object_id ? -1 : (object_id > q.object_id ? 1 : 0)
	end
	
	def evaluate(input)
	  throw "Need to subclass Query.evaluate"
  end
end
