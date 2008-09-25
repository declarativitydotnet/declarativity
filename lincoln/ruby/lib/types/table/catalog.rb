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
    SIZE = 2
    LIFETIME = 3
    KEY = 4
    TYPES = 5
    OBJECT = 6
  end # Field

  @@SCHEMA = [
    TableName.class, # Name
    String.class,    # Table type
    Integer.class,   # The table size
    Float.class,     # The lifetime
    Key.class,       # The primary key
    TypeList.class,  # The type of each attribute
    BasicTable.class      # The table object
  ]

  def initialize # Catalog
    super(TableName.new(GLOBALSCOPE, "catalog"), @@PRIMARY_KEY, TypeList.new(@@SCHEMA))
  end

  protected
    def insert_tup(tuple)
      table = tuple.value(Field::OBJECT)
      if table.nil?
        name = tuple.value(Field::TABLENAME)
        type = tuple.value(Field::TYPE)
        size = tuple.value(Field::SIZE)
        lifetime = tuple.value(Field::LIFETIME)
        key      = tuple.value(Field::KEY)
        types    = tuple.value(Field::TYPES)

        if type == TableType::TABLE
          if (size.to_f == INFINITY) and (lifetime.to_f == INFINITY)
            table = RefTable.new(name,key,types)
          else
            table = BasicTable.new(name, size, lifetime, key, types)
          end
        else
          raise "Don't know how to create table type " + type.to_s
        end
      end
      tuple.value(Field::OBJECT)
      return super(tuple)
    end #Catalog.insert
end # Catalog
