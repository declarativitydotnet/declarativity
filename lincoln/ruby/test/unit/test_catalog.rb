require "lib/types/table/catalog"
require "lib/types/table/table"
require "lib/types/basic/tuple"
require 'lib/core/runtime.rb'
require "test/unit"
require "rubygems"

class TestCatalog < Test::Unit::TestCase
  def default_test
    r = Runtime.new
    initial_card = r.catalog.cardinality
    
    otup = Tuple.new("Orli", Table::TableType::TABLE, (0), TypeList.new([Integer,Float]), nil)     
    dtup = Tuple.new("Dahlia", Table::TableType::TABLE, (0), TypeList.new([Integer,Float]), nil)
    ts = TupleSet.new("tests", otup, dtup)
    r.catalog.insert(ts, nil)
    
    atup = Tuple.new("Adene", Table::TableType::EVENT, (0), TypeList.new([Integer,Float]), nil)
    ts = TupleSet.new("moretests", atup)
    assert_raise(RuntimeError) {r.catalog.insert(ts, nil)}
    
    # catalog now has 2 new entries, for Orli and Dahlia.
    assert_equal(r.catalog.cardinality, 2 + initial_card) 
  end
end
