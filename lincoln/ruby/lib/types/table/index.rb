require "lib/types/table/key.rb"
require "lib/types/table/table_name.rb"
require "lib/types/exception/update_exception.rb"
require "lib/types/basic/type_list.rb"
require "lib/types/table/listener"

class Index
  include Comparable
  include Enumerable

  class Type
    PRIMARY=1
    SECONDARY=2
  end

  @@PRIMARY = 0
  @@SECONDARY = 1

  def initialize(table, key, type)
    @table = table
    @key = key
    @type = type

    iTable = $index
    unless iTable.nil? 
      iTable.insert_tup(Tuple.new(table.name, key, type, self.class, self))
    end
    @table.register_callback(Listener.new(self))
  end

  def to_s
    raise "subclass method for Index.to_s not defined"
  end

  def <=>(i)
    return @table.name <=> i.table.name
  end

  attr_reader :table, :key, :type


  def clear
    raise "subclass method for Index.clear not defined"
  end

  def lookup(tup)
    raise "subclass method for Index.lookup not defined"
  end

  def lookup_kt(the_key, t)
    raise "subclass method for Index.lookup_kt not defined"
  end

   def lookup_vals(*values)
     raise "subclass method for Index.lookup_vals not defined"
   end
  
  def insert(t)
    raise "subclass method for Index.insert not defined"
  end

  def remove(t)
    raise "subclass method for Index.remove not defined"
  end
end
