require "lib/types/exception/update_exception"
require 'lib/types/basic/tuple'
require 'csv'
require 'fileutils'
require 'tmpdir'

class Table
  GLOBALSCOPE = "global"
  INFINITY = 1.0/0

  attr_reader name
  
  def initialize(name, type, size, lifetime, key, attributeTypes)
    @name = name
    @table_type = type
    @attributeTypes = attributeTypes
    @size = size
    @lifetime = lifetime
    @key = key
    @callbacks = Array.new    

	# (pa) catalog->$catalog
    if not $catalog.nil?
      Table.register(name, type, size, lifetime, key, attributeTypes, self)
    end
  end

  def Table.init
    $catalog = Catalog.new
    $index = IndexTable.new
    register($catalog.name, $catalog.table_type, $catalog.size, $catalog.lifetime, $catalog.key, $catalog.types.clone, $catalog)
  end
  
  def catalog
    $catalog
  end
  
  def index
    $index
  end

  class Event
    NONE = 1
    INSERT = 2
    DELETE = 3
  end

  module TableType
    TABLE = 1
    EVENT = 2
    FUNCTION = 3
  end

  def to_s
    value = @name.to_s + ", " + @attributeTypes.to_s + 
    ", " + @size.to_s + ", " + @lifetime.to_s + ", keys(" + @key.to_s + "), {" +
    @attributeTypes.to_s + "}\n"
    if not tuples.nil?
      tuples.each do |t|
        value += t.to_s + "\n"
      end
    end
    return value
  end

  def cardinality
    raise "No subclass definition for cardinality"
  end

  attr_reader :table_type, :name, :size, :lifetime, :key
  
  def types
    @attributeTypes
  end
  
  def tuples
    raise "subclass method for Table.tuples not defined"
  end
  

  # register a new callback on table updates
  def register_callback(callback)
    @callbacks << callback
  end

  def unregister(callback)
    @callbacks.delete(callback)
  end

  def Table.register(name, type, size, lifetime, key, types, object)
    # make sure table wasn't already registered!
    tups = $catalog.primary.lookup_vals(name)
    unless tups.nil? or tups.size == 0
      raise "table " + name.to_s + " already registered\n"
      return
    end
    
    tuple = Tuple.new(name, type.to_s, size, lifetime, key, types, object)
    $catalog.force(tuple)
  end

  def drop
    tuples = catalog.primary.lookup_vals(name)
    retval = (catalog.delete(tuples.tups).size > 0)
    tuples = catalog.primary.lookup_vals(name)
    unless tuples.nil? or tuples.size == 0
      raise "Table.drop failed" 
    end
  end

  def Table.find_table(name)
    raise "Catalog missing" if $catalog.nil?
    tables = $catalog.primary.lookup_vals(name)
    return nil if tables.nil? or tables.size == 0
    return tables.tups[0].value(Catalog::Field::OBJECT) if tables.size == 1
    throw tables.size.to_s + " tables named " + name.to_s + " defined!"
  end

  def <=>(o)
    return (name <=> (o.name))
  end


  def primary
    raise "subclass method for Table.primary not defined"
  end

  def secondary
    raise "subclass method for Table.secondary not defined"
  end

  def insert(tuples, conflicts)
    delta = TupleSet.new(name, nil)
    tuples.each do |t|
      t = t.clone
      if insert_tup(t)
        delta << t
        conflicts += primary.lookup(t) unless (conflicts.nil? or primary.nil?)
      end
    end
    if delta.size > 0
      @callbacks.each {|c| c.insertion(delta)}
    end

    return delta
  end
  
  def clear
    @tuples.clear
    @callbacks.each {|c| c.clear}
  end
  
  def delete(tuples)
    delta = TupleSet.new(name)
    tuples.each do |t|
#      t = t.clone
      if delete_tup(t)
        delta << t
      end
    end

    if delta.size > 0
      @callbacks.each {|c| c.deletion(delta)}
    end
    
    delta.each do |t|
      matches = primary.lookup(t)
      unless matches.nil? or matches.size == 0
        raise "deleted tuple still in primary index" 
      end
    end
    return delta
  end


  def force(tuple)
    insertion = TupleSet.new(name, nil)
    insertion << tuple
    conflicts = TupleSet.new(name, nil)
    insert(insertion, conflicts)
  end

#  protected
  def insert_tup(tuple)
    raise "subclass method for AbstractTable.insert(tuple) not defined"
  end

  def delete_tup(tuple)
    raise "subclass method for AbstractTable.delete_tup(tuple) not defined"
  end
  
  def dump_to_tmp_csv
    dir = "/tmp/lincoln"+Process.pid.to_s
    Dir.mkdir(dir) unless File.exist?(dir) 
    outfile = File.open(dir + "/" + name.to_s+".csv", "w")
    CSV::Writer.generate(outfile, ',') do |csv|
      csv << tuples.tups[0].schema.variables unless (tuples.tups[0].nil? or tuples.tups[0].schema.nil?)
      tuples.each  {|t| csv << t.values}
    end
  end
end
