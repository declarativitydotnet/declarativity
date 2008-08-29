require "lib/types/basic/tuple"
require "lib/types/basic/tuple_set"
require "lib/types/basic/type_list"
require "lib/types/exception/update_exception"

class TableFunction < Table

  def initialize(name, types)
    super(TableName.new(GLOBALSCOPE, name), TableType::FUNCTION, INFINITY, INFINITY, nil, types);
  end

  def insert(tuples, conflicts) 
    raise UpdateException.new
  end

  def insert_tup(t) 
    raise UpdateException.new("Can't remove tuples from a table function")
  end

  def delete(t)
    raise UpdateException.new("Can't remove tuples from a table function")
  end

  def primary
    nil
  end

  def secondary
    nil
  end

  def tuples
    nil
  end

  def cardinality
    return 0;
  end
end
