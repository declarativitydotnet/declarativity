require "lib/types/table/catalog"
require "lib/types/table/table"
require "lib/types/basic/tuple"
require "lib/types/table/index_table"
require "test/unit"
require "rubygems"

class TestCatalog < Test::Unit::TestCase
  def default_test
    $catalog = Catalog.new
    $index = IndexTable.new
    
    otup = Tuple.new("Orli", Table::Type::TABLE, Table::INFINITY,
                     Table::INFINITY, (0), TypeList.new([Integer,Float]),
                      nil)     
    dtup = Tuple.new("Dahlia", Table::Type::TABLE, Table::INFINITY, 50, (0),
                      TypeList.new([Integer,Float]), nil)
    ts = TupleSet.new("tests", otup, dtup)
    $catalog.insert(ts, nil)
    
    atup = Tuple.new("Adene", Table::Type::EVENT, Table::INFINITY, 50, (0),
                      TypeList.new([Integer,Float]), nil)
    ts = TupleSet.new("moretests", atup)
    
    assert_raise(RuntimeError) {$catalog.insert(ts, nil)}
    
    # catalog now has 5 entries:
    # 1 for the catalog
    # 2 each for Orli and Dahlia since we had the last field of their
    #    Catalog insertion tuples set to nil
    assert_equal($catalog.cardinality, 5) 
  end
end