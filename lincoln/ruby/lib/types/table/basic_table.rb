require "lib/types/table/table"
require "lib/types/basic/tuple_set"
require "lib/types/table/hash_index"

class BasicTable < Table
  def initialize(context, name, key, types)
    @tuples = TupleSet.new(name, nil)
    super(name, TableType::TABLE, key, types)
    @key = key
    @primary = HashIndex.new(context, self, key, Index::Type::PRIMARY)
    @secondary = Hash.new
    @context = context
  end
  
  def tuples
    return (@tuples.nil? ? TupleSet.new(name) : @tuples.clone)
  end
  
  attr_reader :primary, :secondary

  def cardinality
    @tuples.size
  end
  
  def insert_tup(tuple)
    @tuples << tuple
    return tuples
  end
  def delete_tup(tuple)
    return @tuples.delete(tuple)
  end
end