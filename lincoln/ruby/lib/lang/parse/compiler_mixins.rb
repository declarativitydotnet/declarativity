# custom declarations for catalog classes
# that are not fully defined automatically

require 'lib/lang/plan/object_from_catalog'

####
#### Below here are mixins that customize catalog classes
####


module AssignmentTableMixin
  include ObjectFromCatalog
end

module CompilerTableMixin
  attr_reader :context
  def initialize_mixin(context)
    @context = context
  end
  def insert_tup(tuple)
    program = tuple.values[field("PROGRAM")]
    if (program.nil?)
      owner = tuple.values[field("OWNER")]
      file  = tuple.values[field("FILE")]
      # require 'ruby-debug'; debugger if @context.nil?
      compiler = Compiler.new(@context, owner, file)
      tuple.set_value(field("NAME"), compiler.the_program.name)
      tuple.set_value(field("PROGRAM"), compiler.the_program)
    end
    return super(tuple)
  end
end

module FactTableMixin
  def initialize_mixin(context)
    programKey = Key.new(field("PROGRAM"))
    index = HashIndex.new(context, self, programKey, Index::Type::SECONDARY)
    @secondary[programKey] = index
  end
  
  module_function:initialize_mixin
end

module IndexTableMixin
  attr_reader :context
  def initialize_mixin(context)
    @context = context
  end
  def insert_tup(tuple)
    if tuple.values[field("OBJECT")].nil? then
      begin
        ctype = tuple.values[field("CLASSNAME")]
        name = tuple.values[field("TABLENAME")]
        if (table = @context.catalog.table(name)).nil?
          raise UpdateException, "can't create index; table " + name.to_s + " not found in catalog"
        end
        # create the index object
        key = tuple.values[field("KEY")]
        type = tuple.values[field("TYPE")]
        index = ctype.new(@context, table, key, type)
        tuple.set_value(field("OBJECT"), index)
      rescue => boom
        raise UpdateException, boom
      end
    end
    return super(tuple)
  end
end


module RuleTableMixin
  include ObjectFromCatalog

  def initialize_mixin(context)
    programKey = Key.new(field("PROGRAM"))
    index = HashIndex.new(context, self, programKey, Index::Type::SECONDARY)
    @secondary[programKey] = index
  end
  
  module_function:initialize_mixin
end

module SelectionTableMixin
  include ObjectFromCatalog
end

module WatchTableMixin
  def watched(program, name, modifier)
#    # require 'ruby-debug'; debugger if program == 'path'  and modifier == 4
    key = Tuple.new(program, name, modifier)
#    puts("checking for watch on [#{program}, #{name}, #{modifier}]") if program == 'path'
    tuples = primary.lookupByKey(primary.key().project(key).values)		
#    tuples = Program.watch.primary.lookup(key)
    if (tuples.size() > 0) then
#      # require 'ruby-debug'; debugger
      return tuples.tups[0].values[field("OPERATOR")]
    end
  end
end

module FunctionTableMixin
  def insert_tup(tuple)
		object = tuple.values[field("OBJECT")]
		raise UpdateException, "Predicate object nil in TableFunction" if object.nil?
		
		object.program   = tuple.values[field("PROGRAM")]
		object.rule      = tuple.values[field("RULE")]
		object.position  = tuple.values[field("POSITION")]
		return super(tuple)
	end
end