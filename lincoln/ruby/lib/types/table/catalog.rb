require "lib/types/table/object_table.rb"
require "lib/types/table/ref_table.rb"
require "lib/types/table/key.rb"
require "lib/types/table/table_name.rb"
require "lib/types/basic/type_list.rb"

class Catalog < ObjectTable
  @@PRIMARY_KEY = Key.new(0)

  class Field # hack to simulate a Java or C enum
    TABLENAME = 0
    TYPE = 1
    KEY = 2
    TYPES = 3
    OBJECT = 4
  end # Field

  @@SCHEMA = [
    TableName.class, # Name
    String.class,    # Table type
    Key.class,       # The primary key
    TypeList.class,  # The type of each attribute
    BasicTable.class      # The table object
  ]

  def initialize(context) # Catalog
    super(context, TableName.new(GLOBALSCOPE, "catalog"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
    @context = context
    @index = nil
  end
  
  def insert_tup(tuple)
    table = tuple.value(Field::OBJECT)
    if table.nil?
      name = tuple.value(Field::TABLENAME)
      type = tuple.value(Field::TYPE)
      key      = tuple.value(Field::KEY)
      types    = tuple.value(Field::TYPES)

      if type == TableType::TABLE
        table = BasicTable.new(@context, name, key, types)
      else
        raise "Don't know how to create table type " + type.to_s
      end
    end
    tuple.value(Field::OBJECT)
    return super(tuple)
  end #Catalog.insert

  #     /**
  # * The index table.
  # * @return The index table.
  # */
  attr_accessor :index

  # /**
  #  * Drop the table with the given name.
  #  * @param name The name of the table.
  #  * @return true if table was dropped, false otherwise.
  #  * @throws UpdateException
  #  */
  def drop(name)
    tuples = primary.lookup_vals(name)
    return delete(tuples).size > 0
  end

  # /**
  #  * Get the table object with the given name.
  #  * @param name The name of the table.
  #  * @return The table object or null if !exist.
  #  */
  def table(name)
    table = primary.lookup_vals(name)

    return nil if table.nil?

    if (table.size() == 1)
      return table.tups[0].value(Catalog::Field::OBJECT)
    elsif (table.size() > 1)
      require 'ruby-debug'; debugger
      raise "Should be one " + name.to_s + " table defined, but there are "+ table.size.to_s + "!"
    end
  end

  # /**
  #  * Register the given table with the catalog.
  #  * @param table The table to be registered.
  #  * @return true if table registration suceeds, false otherwise.
  #  */
  def register(table)
 #   require 'ruby-debug'; debugger if table.name.scope == "path"
#    print "Catalog.register(" + table.name.to_s + ")\n"
    tuple = Tuple.new(table.name, table.table_type, table.key, table.types, table)

    force(tuple);
    return true;
  end
end # Catalog
