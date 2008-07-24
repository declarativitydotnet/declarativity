require "lib/types/table/table"
require "lib/types/basic/tuple"
require "lib/types/table/key"
require "lib/types/table/catalog"
require "lib/types/table/index"
require "lib/types/table/index_table"
require "lib/types/table/listener"
require "test/unit"
require "rubygems"

class TestIndex < Test::Unit::TestCase
  def default_test
    sys = System.new
    sys.init
    
    bt = BasicTable.new('Orli', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    bti = Index.new(bt, Key.new(0), Index::Type::SECONDARY)
    
    # undefined superclass interfaces
    assert_raise(RuntimeError){bti.to_s}
    assert_raise(RuntimeError){bti.clear}
    assert_raise(RuntimeError){bti.lookup(Tuple.new(1))}
    assert_raise(RuntimeError){bti.lookup_kt(Key.new(1), Tuple.new(1))}
    assert_raise(RuntimeError){bti.lookup_vals(1)}
    assert_raise(RuntimeError){bti.insert(Tuple.new(1))}
    assert_raise(RuntimeError){bti.remove(Tuple.new(1))}
    
    assert_equal(bti.key.attributes, [0])
    assert_equal(bti.index_type, Index::Type::SECONDARY)
    assert_equal(bti <=> bti, 0)
    
    btii = IndexTable.new
    
    # attempt to insert an index tuple or a non-existent table
    assert_raise(UpdateException){btii.insert_tup(Tuple.new("Test", Key.new(0), Integer, Index, nil))}
    
    # attempt to insert an index tuple for a pre-existing table
    btii.insert_tup(Tuple.new("Orli", Key.new(0), Integer, Index, nil))
    btii.delete_tup(Tuple.new(1))
    
    # attempt to insert an index tuple when there's two tables of 
    # the same name in the catalog
    bt2 = BasicTable.new('Orli', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    bti2 = Index.new(bt, Key.new(0), Index::Type::SECONDARY)
    assert_raise(UpdateException){btii.insert_tup(Tuple.new("Orli", Key.new(0), Integer, Index, nil))}
    
  end
end