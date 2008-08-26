# custom declarations for catalog classes
# that are not fully defined automatically

## The following module is reused in a bunch of places.  
## Upon tuple insertion, it sets up the object field of 
## tuple with a class instance created from the
## descriptions in the catalog tuple.
## i.e. it will set up the Foo.object attribute of tuple to be
## an instance of the Foo class whose instance vars are filled in from
## the tuple's fields
module ObjectFromCatalog
  def camelize(str)
    str = str.split('_')
    retval = str[0]
    str[1..str.length].each { |s| retval += s.to_s.capitalize }
    return retval
  end
  
  def constants
    retval = Array.new
    classname =self.class.to_s + "::Field"
    consts = eval(classname).constants
    consts.each do |c|
      str = classname + "::" + c
      retval << [c, eval(str)]
    end
    return retval
  end

  def insert_tup(tuple)
    # needs to be done through eval since nested Field is class-dependent
	  object_position = eval self.class.to_s + "::Field::OBJECT"
    object = tuple.value(object_position)
    raise UpdateException, "Object nil in catalog tuple" if object.nil?
    constants.each do |c|
      method = camelize(c[0].downcase) + "="
      object.send method.to_sym, tuple.value(c[1]) unless c[0] == 'OBJECT'
    end
    return super(tuple)
  end
end

####
#### Below here are mixins that customize catalog classes
####


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