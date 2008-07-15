class Program
  include Comparable
	
  class ProgramTable < ObjectTable
		@@PRIMARY_KEY = Key.new(0)
		
    class Field
      PROGRAM = 0
      OWNER = 1
      OBJECT = 2
    end
    
		@@SCHEMA =  [String, String, Program]

		def initialize
			super(TableName.new(@@GLOBALSCOPE, "program"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA));
		end
	end
	
  @@program = ProgramTable.new
  @@rule = RuleTable.new
  @@watch = WatchTable.new
  @@fact = FactTable.new
  @@predicate = PredicateTable.new
  @@tfunction = TableFunction.new
  @@selection = RuleTable.new
  @@assignment = AssignmentTable.new
                        
	def initialize(name, owner) 
		@name        = name
		@owner       = owner
		@definitions = Array.new
		@queries     = Hash.new
		@periodics   = TupleSet.new($system.periodic.name)
		@@program.force(Tuple.new(@name, @owner, self))
		$system.program_put(name, self)
  end	
  
	def tuple
		return Tuple.new(@name, self)
	end
	
	def to_s
		"PROGRAM " + @name;
	end
	
	def definition(table)
		definitions.each do |current|
			if (current.name == table.name) then
				definitions.remove(current)
				break
			end
		end
		@definitions << table
	end
	
	def definitions
		@definitions
	end
	
  def plan
    @queries.clear
    @periodics.clear

    # First plan out all the rules
    rules = rule.secondary[Key.new(RuleTable.Field.PROGRAM).lookup(@name)]

    rules.each do |tuple| 
      rule = tuple.value(RuleTable.Field.OBJECT)

      # Store all planned queries from a given rule. 
      # NOTE: delta rules can produce > 1 query. 
      rule.query(@periodics).each do |query|
        input = query.input
        queries.put(input.name, Array.new) if !queries.containsKey(query.input.name)
        queries[input.name] << query
      end

    end
    if (periodics.size > 0) then
      @periodics.each do |tuple|
        $system.periodic.force(tuple)
      end
    end
    return true;
  end

	def <=>(o) 
		@name.<=>(o.name)
	end

	def queries(name)
		@queries[name]
	end

	def periodics
		@periodics;
	end

  def name
		@name
	end
end


