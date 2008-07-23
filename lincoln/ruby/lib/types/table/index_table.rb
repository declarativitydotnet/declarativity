require "lib/types/table/key"
require "lib/types/table/table_name"
require "lib/types/basic/type_list"
require "lib/types/table/object_table"

class IndexTable < ObjectTable
  @@PRIMARY_KEY = Key.new(0,1)
  @@SCHEMA = [TableName, Key, TableType, String, Index]
  class Field
    TABLENAME = 0
    KEY = 1
    TYPE = 2
    CLASSNAME = 3
    OBJECT = 4
  end

  def initialize
    super(TableName.new(GLOBALSCOPE, "index"), Table::TableType::TABLE, TypeList.new(@@SCHEMA))
  end

  
  def insert_tup(tuple)
    if tuple.value(Field::OBJECT).nil? then
      begin
        ctype = tuple.value(Field::CLASSNAME)
        name = tuple.value(Field::TABLENAME)
        if (table = self.table(name)).nil?
          raise UpdateException, "can't create index; table " + name.to_s + " not found in catalog"
        end
        # create the index object
        key = tuple.value(Field::KEY)
        type = tuple.value(Field::TYPE)
        index = ctype.new(table, key, type)
        tuple.set_value(Field::OBJECT, index)
      rescue => boom
        raise UpdateException, boom
      end
    end
    return super(tuple)
  end
  def delete_tup(tuple)
    super(tuple)
  end
  
  def table(name)
    return Table.table(name)
  end
  
end # Index.IndexTable
