require "lib/types/table/table"
require "lib/types/table/index_table"
require "lib/types/basic/tuple"
require "lib/types/table/key"
require "lib/types/table/catalog"
require "lib/types/table/hash_index"
require "lib/types/table/callback"
require "test/unit"
require "rubygems"

class TestTable < Test::Unit::TestCase
  def default_test
    $catalog = Catalog.new
    $index = IndexTable.new

    t = Table.new('Orli', 10, Table::INFINITY, Table::INFINITY, Key.new(0), [Integer, String])
    
    # undefined superclass methods
    assert_raise(RuntimeError){t.tuples}
    assert_raise(RuntimeError){t.cardinality}
    assert_raise(RuntimeError){t.primary}
    assert_raise(RuntimeError){t.secondary}
    
    assert_equal(t.types.to_s, "IntegerString")
    assert_equal(t.size, Table::INFINITY)
    assert_equal(t.lifetime, Table::INFINITY)

    # silly coverage for unregister
    t.unregister(Callback.new(t))
    
    assert_raise(RuntimeError){t.insert_tup(Tuple.new(1, 'hi'))}
    assert_raise(RuntimeError){t.delete_tup(Tuple.new(1, 'hi'))}
  end
end