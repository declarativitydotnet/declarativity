require 'lib/lang/plan/clause'
require 'lib/types/table/object_table'
require 'lib/types/operator/watch_op'
class WatchClause < Clause
	
	class WatchTable < ObjectTable
		@@PRIMARY_KEY = Key.new(0,1,2)

		class Field 
		  PROGRAM=1,
		  TABLENAME=2,
		  MODIFIER=3, 
		  OPERATOR=4
	  end
		@@SCHEMA =  [String, TableName, WatchOp::Modifier, WatchOp]
      # String.class,                             // Program name
      # TableName.class,                          // Table name
      # p2.types.operator.Watch.Modifier.class,   // Modifier
      # p2.types.operator.Watch.class             // Operator

		def initialize
			super(TableName.new(GLOBALSCOPE, "watch"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
		end
		
		def watched(program, name, modifier)
			key = Tuple.new(program, name, modifier)
			tuples = Program.watch.primary.lookup(key)
			if (tuples.size() > 0) then
					return tuples.iterator.next.value(Field::OPERATOR)
			end
		end
	end
	
	def initialize(location, name, modifier)
		super(location)
		@name = name
		@modifier = modifier
	end
	
	def to_s
		return "watch(" + name + ", " + modifier + ")."
	end

	def set(program)
		Compiler.watch.force(Tuple.new(program, name, modifier, Watch.new(program, null, name, modifier)))
	end
end
