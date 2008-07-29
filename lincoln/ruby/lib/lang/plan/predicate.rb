require 'lib/lang/plan/term'
require 'lib/lang/plan/dont_care'
require 'lib/lang/plan/arguments'
require 'lib/types/operator/index_join'
require 'lib/types/operator/scan_join'
require 'lib/lang/plan/object_from_catalog'

class Predicate < Term 
  include Enumerable
  
	class PredicateTable < ObjectTable
	  include ObjectFromCatalog

    class Field
      PROGRAM = 0
      RULE = 1
      POSITION = 2 
      EVENT = 3
      OBJECT = 4
    end
	  
		@@PRIMARY_KEY = Key.new(0,1,2)
		
		@@SCHEMA =  [
			String.class,     # program name
			String.class,     # rule name
			Integer.class,    # position
			String.class,     # Event
			Predicate.class   # predicate object
		]

		def initialize
			super(TableName.new(GLOBALSCOPE, "predicate"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
		end
	end
	
	def initialize(notin, name, event, arguments) 
		super()
		@notin = notin
		@name = name
		@event = event
		@arguments = Arguments.new(self, arguments)
	end

	attr_reader :schema, :notin, :name
	attr_accessor :event
	
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
		value = (notin ? "notin " : "") + name.to_s + "("
		if @arguments.length == 0
			return value + ")"
		end
		value += argument(0).to_s
		(1..@arguments.length-1).each do |i|
			value += ", " + argument(i).to_s
		end
		value += argument(@arguments.length).to_s
		return value + ")"
	end

	def requires
		variables = Hash.new
		@arguments.each do |a|
			if (!(a.class <= Variable)) then
			  a.variables.each{|v| variables << v}
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
			return AntiScanJoin.new(self, input)
		end
		
		table = Table.find_table(@name)
		index = nil
		if (indexKey.size > 0) then
			if (table.primary.key == indexKey) then
				index = table.primary
			elsif (table.secondary.has_key?(indexKey))
				index = table.secondary[indexKey.hash]
			else
				index = HashIndex.new(table, indexKey, Index::Type::SECONDARY)
				table.secondary[indexKey.hash] = index
			end
		end
		
		if !index.nil? then
			return IndexJoin.new(self, input, lookupKey, index)
		else
			return ScanJoin.new(self, input)
		end
	end
	
	def set(program, rule, position) 
#    Program.predicate.force(Tuple.new(program, rule, position, event.to_s, self))
    Program.predicate.force(Tuple.new(program, rule, position, event, self))
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
