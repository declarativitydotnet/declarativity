require "lib/types/table/basic_table"
require "lib/types/basic/tuple_set"
require "lib/types/basic/tuple"
require "lib/types/table/key"
require "lib/types/table/catalog"
require "test/unit"
require "rubygems"

class TestHashIndex < Test::Unit::TestCase
  def default_test
    sys = System.new
    sys.init

    bt = BasicTable.new('Orli', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    ts = TupleSet.new('test', Tuple.new(1, 'hi'))
    assert_equal(bt.insert(ts, nil).tups, ts.tups)
    
    assert(bt.primary.to_s =~ /Index Orli\n1TupleSet.*/)
    bt.primary.clear
    assert_equal(bt.primary.to_s, "Index Orli\n\n")    
    
    assert_equal(bt.primary.lookup_kt(bt.key, Tuple.new(1, 'hi')).tups, [])
    assert_raise(NameError) {bt.primary.lookup_vals(1, 2)}
  end
  end