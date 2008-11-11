require 'lib/types/table/object_table'
require 'lib/lang/plan/watch_clause'
require 'lib/lang/plan/fact'
require 'lib/lang/plan/predicate'
require 'lib/lang/plan/function'
require 'lib/lang/plan/assignment'
require 'lib/lang/plan/rule'
require 'lib/lang/parse/schema'
require 'lib/core/periodic'

class Program
  include Comparable
		
#   @@program = MyProgramTable.new(context)
# #  @@rule = Rule::RuleTable.new
#   # @@watch = WatchTable.new
#   @@fact = MyFactTable.new(context)
#   # @@predicate = PredicateTable.new
#   # @@tfunction = Function::TableFunction.new
#   @@selection = MySelectionTable.new(context)
#   @@assignment = MyAssignmentTable.new(context)
  
  def Program.watch
    @@watch
  end    
  
  def Program.predicate
    @@predicate
  end
  def Program.selection
    @@selection
  end
              
	def initialize(context, name, owner) 
	  @context     = context
		@name        = name
		@owner       = owner
		@definitions = Array.new
		@queries     = Hash.new
		@periodics   = TupleSet.new(Periodic.table_name)
		context.catalog.table(ProgramTable.table_name).force(Tuple.new(@name, @owner, self))
#		System.install_program(name, self)
  end	
  
	def tuple
		return Tuple.new(@name, self)
	end
	
	def to_s
		"PROGRAM " + @name;
	end
	
	def definition(table)
		definitions.each_with_index do |current, i|
			if (current.name == table.name) then
				definitions.delete_at(i)
				break
			end
		end
		@definitions << table
	end
	
	attr_reader :definitions, :periodics, :name, :queries
	
  def plan
    @queries = Hash.new
    @periodics.clear

    # First plan out all the rules
#    require 'ruby-debug'; debugger
    rules = @context.catalog.table(RuleTable.table_name).secondary[Key.new(RuleTable::Field::PROGRAM)].lookup_vals(@name)

    unless rules.nil? then
      rules.each do |tuple| 
        rule = tuple.value(RuleTable::Field::OBJECT)

        # Store all planned queries from a given rule. 
        # NOTE: delta rules can produce > 1 query. 
        rule.query(@context, @periodics).each do |query|
         #  require 'ruby-debug'; debugger
          input = query.input
          @queries[input.name.hash] = Array.new if !@queries.has_key?(query.input.name.hash)
          @queries[input.name.hash] << query
        end
      end
    end
  
    if (periodics.size > 0) then
      @periodics.each do |tuple|
        @context.catalog.table(Periodic.table_name).force(tuple)
      end
    end
    return true;
  end

	def <=>(o) 
		@name.<=>(o.name)
	end

	def get_queries(name)
		@queries[name.hash]
	end
end


