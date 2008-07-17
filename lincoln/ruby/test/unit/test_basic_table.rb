require "lib/types/table/basic_table"
require "lib/types/basic/tuple_set"
require "lib/types/basic/tuple"
require "lib/types/table/key"
require "lib/types/table/catalog"
require "lib/core/system"
require "test/unit"
require "rubygems"

class TestBasicTable < Test::Unit::TestCase
  def default_test

    bt = BasicTable.new('Orli', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    testtup1 = Tuple.new(1, 'hi')
    testtup2 = Tuple.new(2, 'bye')
    ts = TupleSet.new('test', testtup1, testtup2)
    assert_equal(bt.cardinality, 0)
    assert_equal(bt.delete(ts).tups, [])
    assert_equal(bt.insert(ts, nil).tups, ts.tups)
    assert(!bt.primary.nil?)
    assert(!bt.secondary.nil?)
    assert_equal(bt.tuples.nil?, false)
    assert_equal(bt.cardinality, 2)
    assert_equal(bt.to_s, "Orli, IntegerString, 10, Infinity, keys(0), {IntegerString}<1, hi>\n<2, bye>\n")
    assert_equal(bt.delete(ts).tups, [testtup1, testtup2])
    assert_equal(bt.to_s, "Orli, IntegerString, 10, Infinity, keys(0), {IntegerString}")
    assert_equal(bt.type, Table::Type::TABLE)    
    
    # Todo: check persistence of BasicTable
  end
  end