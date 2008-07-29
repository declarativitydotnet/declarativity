# additional declarations for catalog classes
# that are not fully served by schema.rb
require 'lib/lang/plan/object_from_catalog'

module AssignmentTableMixin
  include ObjectFromCatalog
end

module CompilerTableMixin
  def insert_tup(tuple)
    program = tuple.value(field("PROGRAM"))
    if (program.nil?)
      owner = tuple.value(field("OWNER"))
      file  = tuple.value(field("FILE"))
      compiler = Compiler.new(owner, file)
      tuple.value(field("NAME"), compiler.program.name)
      tuple.value(field("PROGRAM"), compiler.program)
    end
    return super(tuple)
  end
end

module FactTableMixin
  def initialize_mixin
    programKey = Key.new(field("PROGRAM"))
    index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
    @secondary[programKey] = index
  end
  
  module_function:initialize_mixin
end

module IndexTableMixin
  def insert_tup(tuple)
    if tuple.value(field("OBJECT")).nil? then
      begin
        ctype = tuple.value(field("CLASSNAME"))
        name = tuple.value(field("TABLENAME"))
        if (table = Table.find_table(name)).nil?
          raise UpdateException, "can't create index; table " + name.to_s + " not found in catalog"
        end
        # create the index object
        key = tuple.value(field("KEY"))
        type = tuple.value(field("TYPE"))
        index = ctype.new(table, key, type)
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

  def initialize_mixin
    programKey = Key.new(field("PROGRAM"))
    index = HashIndex.new(self, programKey, Index::Type::SECONDARY)
    @secondary[programKey.hash] = index
  end
  
  module_function:initialize_mixin
end

module SelectionTableMixin
  include ObjectFromCatalog
end

module WatchTableMixin
  def watched(program, name, modifier)
    key = Tuple.new(program, name, modifier)
    tuples = Program.watch.primary.lookup(key)
    if (tuples.size() > 0) then
      return tuples.iterator.next.value(field("OPERATOR"))
    end
  end
end