require "lib/types/table/catalog"
require "lib/types/table/table"
require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestCatalog < Test::Unit::TestCase
  def default_test
    $catalog=nil; $index=nil
    sys = System.new
    sys.init
    initial_card = $catalog.cardinality
    
    otup = Tuple.new("Orli", Table::TableType::TABLE, Table::INFINITY,
                     Table::INFINITY, (0), TypeList.new([Integer,Float]),
                      nil)     
    dtup = Tuple.new("Dahlia", Table::TableType::TABLE, Table::INFINITY, 50, (0),
                      TypeList.new([Integer,Float]), nil)
    ts = TupleSet.new("tests", otup, dtup)
    $catalog.insert(ts, nil)
    
    atup = Tuple.new("Adene", Table::TableType::EVENT, Table::INFINITY, 50, (0),
                      TypeList.new([Integer,Float]), nil)
    ts = TupleSet.new("moretests", atup)
    
    assert_raise(RuntimeError) {$catalog.insert(ts, nil)}
    
    # catalog now has 4 new entries, 2 each for Orli and Dahlia.
    # It's 2 each because we had the last field of their
    #    Catalog insertion tuples set to nil
    assert_equal($catalog.cardinality, 4 + initial_card) 
  end
end