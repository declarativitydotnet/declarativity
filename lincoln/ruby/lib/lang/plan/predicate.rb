require 'lib/lang/plan/term'
require 'lib/lang/plan/dont_care'

class Predicate < Term 
  include Enumerable
  class Field
    PROGRAM = 0
    RULE = 1
    POSITION = 2 
    EVENT = 3
    OBJECT = 4
  end
  
	class PredicateTable < ObjectTable
		@@PRIMARY_KEY = Key.new(0,1,2)
		
		@@SCHEMA =  [
			String.class,     # program name
			String.class,     # rule name
			Integer.class,    # position
			String.class,     # Event
			Predicate.class   # predicate object
		]

		def initialize
			super(TableName.new(GLOBALSCOPE, "predicate"), @@PRIMARY_KEY,  TypeList.new(SCHEMA))
		end
		
    def insert(tuple)
			object = tuple.value(Field.OBJECT)
			if (object.nil?) then
				throw UpdateException, "Predicate object null"
			end
			object.program   = tuple.value(Field.PROGRAM)
			object.rule      = tuple.value(Field.RULE)
			object.position  = tuple.value(Field.POSITION)
			return super(tuple);
		end
		
    def delete(tuple) 
			return super(tuple)
		end
	end
	
	def initialize(notin, name, event, arguments) 
		super()
		@notin = notin;
		@name = name;
		@event = event;
		@arguments = Arguments.new(self, arguments);
	end
	
	def schema
		@schema
	end
	
  def notin
		@notin
	end
	
	def event
		@event
	end
	
	def event=(e) 
		@event = e
	end
	
	def name
		@name
	end
	
	def containsAggregation
		arguments.each do |e|
			if e.class <= Aggregate then
				return true;
		  end
	  end
		
		false
	end

	# An iterator over the predicate arguments.
  def each
    @arguments.each do |a|
      yield a
    end
  end
	
	def argument(i)
		return @arguments[i]
	end
	
	# was called "arguments" in the Java code!!
	def num_arguments
		return @arguments.length
	end
	
  def to_s
		value = (notin ? "notin " : "") + name + "("
		if @arguments.length == 0
			return value + ")"
		end
		value += arguments[0].to_s
		(1..@arguments.length).each do |i|
			value += ", " + arguments[i]
		end
		return value + ")"
	end

	def requires
		variables = Hash.new
		@arguments.each do |a|
			if (!(a.class <= Variable)) then
			  a.variables.map{|v| variables << v}
			end
		end
		return variables
	end

  def operator(input) 
		# Determine the join and lookup keys.
		lookupKey = Key.new
		indexKey  = Key.new
		@schema.variables.each do |var|
			if (input.contains(var)) then
				indexKey << var.position
				lookupKey << input.variable(var.name).position
			end
		end
		
		if (@notin) then
			return AntiScanJoin.new(this, input)
		end
		
		table = Table.new(@name)
		index = nil
		if (indexKey.size > 0) then
			if (table.primary.key == indexKey) then
				index = table.primary
			elsif (table.secondary.contains(indexKey))
				index = table.secondary.get(indexKey)
			else
				index = HashIndex.new(table, indexKey, Index.Type.SECONDARY)
				table.secondary.put(indexKey, index)
			end
		end
		
		if !index.nil? then
			return IndexJoin.new(this, input, lookupKey, index)
		else
			return ScanJoin.new(this, input)
		end
	end
	
	def set(program, rule, position) 
## postpone til we get the catalog tables and $program variable going
#		$program.predicate.force(Tuple.new($program, rule, position, event.toString(), this))
		
		@schema = Schema.new(name, nil)
		@arguments.each do |arg|
			if (arg.class <= Variable) then
				@schema << arg
			else 
				@schema << DontCare.new(arg.class)
			end
		end
	end
	
end
