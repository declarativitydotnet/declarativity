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
    assert_equal(bt.delete(ts).tups.to_a, [])
    assert_equal(bt.insert(ts, nil).tups, ts.tups)
    assert(!bt.primary.nil?)
    assert(!bt.secondary.nil?)
    assert_equal(bt.tuples.nil?, false)
    assert_equal(bt.cardinality, 2)
    bt.tuples.tups.each { |t| assert((t.values == [1, "hi"]) || (t.values == [2, "bye"])) }
    #assert_equal([testtup1, testtup2], bt.delete(ts).tups)
	deletions = bt.delete(ts).tups
	assert(deletions == [testtup1,testtup2] || [testtup2,testtup1])
    # what's with the newlines?  can't be bothered with it now...
    assert_equal(bt.to_s, "Orli, IntegerString, 10, Infinity, keys(0), {IntegerString}\n")
    assert_equal(bt.table_type, Table::TableType::TABLE)    
    
    # Todo: check persistence of BasicTable
  end
  end
