require "lib/types/table/table"
require "lib/types/basic/tuple"
require "lib/types/table/key"
require "lib/types/table/catalog"
require "lib/types/table/index"
require "lib/types/table/listener"
require 'lib/core/system'
require "test/unit"
require "rubygems"

class TestIndex < Test::Unit::TestCase
  def default_test
    $catalog=nil; $index=nil
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
        
    # attempt to insert an index tuple or a non-existent table
    assert_raise(UpdateException) do 
      $index.insert_tup(Tuple.new("Test", Key.new(0), Integer, Index, nil))
    end
    
    # attempt to insert an index tuple for a pre-existing table
    $index.insert_tup(Tuple.new("Orli", Key.new(0), Integer, Index, nil))
    $index.delete_tup(Tuple.new(1))

    # This error now caught on Table.new.
    # # attempt to insert an index tuple when there's two tables of 
    # # the same name in the catalog
    # bt2 = BasicTable.new('Orli', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    # bti2 = Index.new(bt, Key.new(0), Index::Type::SECONDARY)
    # assert_raise(UpdateException){$index.insert_tup(Tuple.new("Orli", Key.new(0), Integer, Index, nil))}
    
  end
end